#include "Client.h"

//Class that create a connection and manage request with client, that in our case is ServerHLS

// Define the variables
char * MiddlewareBB::Client::entity; //save file data before return
char * MiddlewareBB::Client::statusline; //status line of response
char * MiddlewareBB::Client::first_ts; //first ts of manifest
char MiddlewareBB::Client::request[5000], MiddlewareBB::Client::response[5000], MiddlewareBB::Client::manifest[100], MiddlewareBB::Client::manifest_path[100],
    MiddlewareBB::Client::ts[100], MiddlewareBB::Client::last_chunk[100], MiddlewareBB::Client::chunk_time[100];
int MiddlewareBB::Client::entity_length, MiddlewareBB::Client::h_num, MiddlewareBB::Client::c_read, MiddlewareBB::Client::tmp, MiddlewareBB::Client::j;
struct sockaddr_in MiddlewareBB::Client::addr;

//Constructor
MiddlewareBB::Client::Client(unsigned long targetip, int port) {

    // Set up OpenSSL
    SSL_library_init();
    MiddlewareBB::Global::ctx = SSL_CTX_new(TLS_client_method());

    //Create socket
    MiddlewareBB::Global::client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == MiddlewareBB::Global::client_sock) {
        tmp = errno; //to stamp errno
        perror("Socket fallita");
        printf("Errno=%d\n", tmp);
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Create connection
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = targetip;
    if (-1 == connect(MiddlewareBB::Global::client_sock, (struct sockaddr * ) & addr, sizeof(struct sockaddr_in))) {
        perror("Connect fallita");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    // Set up SSL/TLS
    MiddlewareBB::Global::ssl = SSL_new(MiddlewareBB::Global::ctx);
    SSL_set_fd(MiddlewareBB::Global::ssl, MiddlewareBB::Global::client_sock);
    if (SSL_connect(MiddlewareBB::Global::ssl) != 1) {
        perror("SSL fallita");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Index request
    MiddlewareBB::Global::file f; //tmp file
    f = getFile(MiddlewareBB::Global::client_index);
    if (f.path == nullptr) { //if not 200 OK
        perror("index 404 not found");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }
    MiddlewareBB::Global::ind = std::make_shared < MiddlewareBB::Global::file > (f);

    //Parse Index --> Find name-path of Manifest (if more than one takes the last)
    int flag = 1;
    for (int i = f.length; i > 3; i--) {
        if ((f.data[i - 4] == '.') && (f.data[i - 3] == 'm') && (f.data[i - 2] == '3') && (f.data[i - 1] == 'u') && (f.data[i] == '8')) {
            flag = 0;
            for (int a = i; a > 1; a--) {

                //Isolate file name
                if ((f.data[a - 1] == ' ') || (f.data[a - 1] == '\n')) {
                    memcpy(manifest, & f.data[a], i - a + 1); //save manifest with path, ex: out/u/bbb/qxa/manifest_3.m3u8
                    memcpy(manifest_path, & f.data[a], i - a + 1); //save only the manifest path, ex: out/u/bbb/qxa/
                    for (int f = strlen(manifest_path); f > 0; f--) {
                        if (manifest_path[f] == '/') {
                            manifest_path[f + 1] = 0;
                            break;
                        }
                    }

                    //Somentimes the path is not specify in index file, in this case is the same path of index
                    if(strcmp(manifest, manifest_path) == 0){
                        memcpy(manifest_path, MiddlewareBB::Global::client_index, strlen(MiddlewareBB::Global::client_index));
                            for (int f = strlen(manifest_path); f > 0; f--) {
                                if (manifest_path[f] == '/') {
                                    manifest_path[f + 1] = 0;
                                    break;
                                }
                            }
                        strcpy(manifest, manifest_path); //add manifest path
                        memcpy(manifest + strlen(manifest_path), & f.data[a], i - a + 1); //path + manifest name
                    }
                    break;
                }
            }
            break;
        }
    }
    if (flag) {
        perror("manifest not found in index file");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Manifest request
    f = getFile(manifest);
    if (f.path == nullptr) { //if not 200 OK
        perror("manifest 404 not found");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }
    //Push on cache_man
    while (!MiddlewareBB::Global::lock_cache_man.try_lock());
    MiddlewareBB::Global::cache_man.push_back(f);
    MiddlewareBB::Global::lock_cache_man.unlock();

    for (int i = f.length; i > 1; i--) {
        //Find last ts file name in manifest
        if ((f.data[i - 2] == '.') && (f.data[i - 1] == 't') && (f.data[i] == 's')) {
            for (int a = i; a > 1; a--) {

                //Isolate file name
                if ((f.data[a - 1] == ' ') || (f.data[a - 1] == '\n')) {
                    strcpy(ts, manifest_path);
                    memcpy(ts + strlen(manifest_path), & f.data[a], i - a + 1);
                    memcpy(last_chunk, ts, strlen(ts));
                    break;
                }
            }
            break;
        }
    }

    //Find #EXTINF:, this is the time of a chunk, so wait will be the half
    for (int i = 7; i < f.length; i++) {
        if ((f.data[i - 7] == '#') && (f.data[i - 6] == 'E') && (f.data[i - 5] == 'X') && (f.data[i - 4] == 'T') &&
            (f.data[i - 3] == 'I') && (f.data[i - 2] == 'N') && (f.data[i - 1] == 'F') && (f.data[i] == ':')) {
            for (int j = i + 1; j < f.length; j++) {
                if ((f.data[j] == '\n') || (f.data[j] == '\r') || (f.data[j] == ',')) {
                    memcpy(chunk_time, & f.data[i + 1], j - i - 1);
                    chunk_time[j - i] = 0;
                    MiddlewareBB::Global::wait = atof(chunk_time) / 2;
                    break;
                }
            }
            break;
        }
    }
}

//Setting the path returns the struct file from the ServerHLS
//Return a File with content of the body or {nullptr,nullptr} if not 200 OK
MiddlewareBB::Global::file MiddlewareBB::Client::getFile(char * path) {

    //Lock socket
    while (!MiddlewareBB::Global::lock_client_sock.try_lock());

    //Request
    sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\n\r\n", path, MiddlewareBB::Global::client_host, MiddlewareBB::Global::client_port);
    if (-1 == SSL_write(MiddlewareBB::Global::ssl, request, strlen(request))) {
        perror("write fallita");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Start reading response and parsing headers
    bzero(h, sizeof(struct header) * 100);
    statusline = h[0].name = response;
    for (j = 0, h_num = 0; SSL_read(MiddlewareBB::Global::ssl, response + j, 1); j++) {
        if (response[j] == ':' && (h[h_num].value == 0)) {
            response[j] = 0;
            h[h_num].value = response + j + 1;
        } else if ((response[j] == '\n') && (response[j - 1] == '\r')) {
            response[j - 1] = 0;
            if (h[h_num].name[0] == 0) {
                break;
            }
            h[++h_num].name = response + j + 1;
        }
    }

    //Read response status code
    for (j = 0; statusline[j] != ' ' && statusline[j] != 0; j++);
    statusline = statusline + j + 1;
    for (j = 0; statusline[j] != ' ' && statusline[j] != 0; j++);
    statusline[j] = 0;
    int status = atoi(statusline);

    if (status != 200) { //if not 200 OK
        MiddlewareBB::Global::lock_client_sock.unlock();
        return {
            nullptr,
            nullptr,
            nullptr
        };
    }

    //Find response body length
    entity_length = -1;
    for (int i = 1; i < h_num; i++) {
        if (strcmp(h[i].name, "Content-Length") == 0) {
            entity_length = atoi(h[i].value);
        }
    }
    if (entity_length == -1) {
        perror("Content-Length not found");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Response body
    entity = (char * ) malloc(entity_length);

    for (j = 0; (entity_length - j != 0) &&
        (c_read = SSL_read(MiddlewareBB::Global::ssl, entity + j, entity_length - j)) > 0; j += c_read);

    MiddlewareBB::Global::lock_client_sock.unlock();

    if (strcmp(path, manifest) == 0) {
        for (int i = 2; i < entity_length; i++) {

            //Find first ts file name in manifest
            if ((entity[i - 2] == '.') && (entity[i - 1] == 't') && (entity[i] == 's')) {
                for (int a = i; a > 1; a--) {

                    //Isolate file name
                    if ((entity[a - 1] == ' ') || (entity[a - 1] == '\n')) {
                        first_ts = (char * ) malloc(strlen(manifest_path) + i - a + 1);
                        strcpy(first_ts, manifest_path);
                        memcpy(first_ts + strlen(manifest_path), & entity[a], i - a + 1);
                        break;
                    }
                }
                break;
            }
        }
        return {
            path,
            entity,
            first_ts,
            entity_length,
            time(nullptr)
        };
    } else {
        return {
            path,
            entity,
            nullptr,
            entity_length,
            time(nullptr)
        };
    }
}

//Thread request manifest and add the last block (.ts) if it's not already requested
int MiddlewareBB::Client::manRequest(void * list) {
    try {
        std::vector < MiddlewareBB::Global::file > * cache_man = (std::vector < MiddlewareBB::Global::file > * ) list;
        MiddlewareBB::Global::file f; //tmp file
        MiddlewareBB::Cache * c = new MiddlewareBB::Cache();

        while (recv(MiddlewareBB::Global::client_sock, NULL, 1, MSG_PEEK | MSG_DONTWAIT) != 0 && MiddlewareBB::Global::running) {
            c -> printCache();
            f = getFile(manifest);
            if (f.path == nullptr) { //if not 200 OK
                perror("manifest 404 not found");
                MiddlewareBB::Global::running = 0;
                exit(1);
            }
            for (int i = f.length; i > 1; i--) {

                //Find last ts file name in manifest
                if ((f.data[i - 2] == '.') && (f.data[i - 1] == 't') && (f.data[i] == 's')) {
                    for (int a = i; a > 1; a--) {

                        //Isolate file name
                        if ((f.data[a - 1] == ' ') || (f.data[a - 1] == '\n')) {
                            strcpy(ts, manifest_path);
                            memcpy(ts + strlen(manifest_path), & f.data[a], i - a + 1);

                            //If is different from the last ts file
                            if (strcmp(ts, last_chunk) != 0) {
                                memcpy(last_chunk, ts, strlen(ts));

                                //Push on cache_man
                                while (!MiddlewareBB::Global::lock_cache_man.try_lock());
                                cache_man -> push_back(f);
                                MiddlewareBB::Global::lock_cache_man.unlock();

                                //Update reference
                                if (MiddlewareBB::Global::reference != nullptr) {
                                    while (!MiddlewareBB::Global::lock_cache_man.try_lock());
                                    for (auto it = cache_man -> cbegin(); it != cache_man -> cend();) {
                                        if (strcmp(it -> first_ts, MiddlewareBB::Global::reference -> first_ts) == 0) {
                                            ++it;
                                            MiddlewareBB::Global::reference = std::make_shared < MiddlewareBB::Global::file > (*it);
                                            break;
                                        } else {
                                            ++it;
                                        }
                                    }
                                    MiddlewareBB::Global::lock_cache_man.unlock();
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            usleep(MiddlewareBB::Global::wait * 1000000);
        }
        MiddlewareBB::Global::running = 0;
    } catch (...) {
        MiddlewareBB::Global::running = 0;
        SSL_shutdown(MiddlewareBB::Global::ssl);
        SSL_free(MiddlewareBB::Global::ssl);
        SSL_CTX_free(MiddlewareBB::Global::ctx);
        close(MiddlewareBB::Global::client_sock);
        throw;
    }
    return 0;
}