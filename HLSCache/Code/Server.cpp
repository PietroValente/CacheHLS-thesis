#include "Server.h"

//Class manage access to cache

// Define the variables
int MiddlewareBB::Server::c, MiddlewareBB::Server::i, MiddlewareBB::Server::j, MiddlewareBB::Server::k,
    MiddlewareBB::Server::s, MiddlewareBB::Server::s2, MiddlewareBB::Server::ka;

socklen_t MiddlewareBB::Server::len;

char * MiddlewareBB::Server::commandline, * MiddlewareBB::Server::method, * MiddlewareBB::Server::path,
    * MiddlewareBB::Server::ver;
char MiddlewareBB::Server::request[5000], MiddlewareBB::Server::response[5000], MiddlewareBB::Server::command[100];
struct sockaddr_in MiddlewareBB::Server::addr, MiddlewareBB::Server::remote_addr;

//Constructor
MiddlewareBB::Server::Server(int port) {
    //Create a socket which will then be the one that will handle the arrival of connections
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Socket Failed");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Create the structure addr
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = 0;

    //Set the socket options
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int)) == -1) {
        perror("Setsockopt Failed");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Assigns the address specified by addr to the socket referred
    if (bind(s, (struct sockaddr * ) & addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Bind Failed");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }

    //Marks the socket referred to by sockfd as a passive socket
    if (listen(s, 225) == -1) {
        perror("Listen Failed");
        MiddlewareBB::Global::running = 0;
        exit(1);
    }
    len = sizeof(struct sockaddr_in);
}

//Thread that accepts connections and creates a new thread for each new one
int MiddlewareBB::Server::listener(void * ca) {
    try {
        std::vector < MiddlewareBB::Global::file > * cache = (std::vector < MiddlewareBB::Global::file > * ) ca;

        while (MiddlewareBB::Global::running) {
            //New socket that accept first connection form s list
            s2 = accept(s, (struct sockaddr * ) & remote_addr, & len);

            //Allocate stack for child task.
            const int STACK_SIZE = 4096;
            char * stack = (char * ) malloc(STACK_SIZE);
            if (!stack) {
                perror("Malloc request fail");
                MiddlewareBB::Global::running = 0;
                exit(1);
            }

            //Create a new thread for each connection
            if (clone(requestManager, stack + STACK_SIZE, CLONE_VM, cache) == -1) {
                perror("Clone requestManager");
                MiddlewareBB::Global::running = 0;
                exit(1);
            }
        }
        MiddlewareBB::Global::running = 0;
    } catch (...) {
        MiddlewareBB::Global::running = 0;
        throw;
    }
    return 0;
}

int MiddlewareBB::Server::requestManager(void * ca) {
    try {
        std::vector < MiddlewareBB::Global::file > * cache = (std::vector < MiddlewareBB::Global::file > * ) ca;

        //Socket value control
        if (s2 == -1) {
            perror("Accept Failed");
            MiddlewareBB::Global::running = 0;
            exit(1);
        }
        do {
            //Parsing of request headers
            bzero(h, 100 * sizeof(struct header * ));
            commandline = h[0].n = request;

            //Lock socket
            while (!MiddlewareBB::Global::lock_server_sock.try_lock());
            int first_line = 1;
            for (j = 0, k = 0; read(s2, request + j, 1); j++) {

                //Print all the request in debugging mode
                if (MiddlewareBB::Global::log) {
                    printf("%c", request[j]);
                }
                if (request[j] == ':' && (h[k].v == 0) && !(first_line)) {
                    request[j] = 0;
                    h[k].v = request + j + 1;
                } else if ((request[j] == '\n') && (request[j - 1] == '\r')) {
                    first_line = 0;
                    request[j - 1] = 0;
                    if (h[k].n[0] == 0) break;
                    h[++k].n = request + j + 1;
                }
            }
            MiddlewareBB::Global::lock_server_sock.unlock();
            if (MiddlewareBB::Global::log) {
                printf("\n");
            }

            //Verify request expect a keep-alive connection
            ka = 0;
            for (i = 1; i < k; i++) {
                if (strcmp(h[i].n, "Connection") == 0) {
                    ka = !strcmp(h[i].v, " keep-alive");
                }
            }

            //Parsing command line, example: "GET /chunks/manifest.m3u8 HTTP/1.1"
            //method = GET
            //path = /chunks/manifest.m3u8
            //ver = HTTP/1.1
            method = commandline;
            for (i = 0; commandline[i] != ' '; i++);
            commandline[i] = 0;
            path = commandline + i + 1;
            for (i++; commandline[i] != ' '; i++);
            commandline[i] = 0;
            ver = commandline + i + 1;

            while (!MiddlewareBB::Global::lock_server_sock.try_lock());

            //Try to open file requested
            if (strcmp(path + 1, MiddlewareBB::Global::ind -> path) == 0) {
                if (MiddlewareBB::Global::reference == nullptr) {
                    if (MiddlewareBB::Global::cache_man.size() > MiddlewareBB::Global::default_ref) {
                        MiddlewareBB::Global::reference = std::make_shared < MiddlewareBB::Global::file > (MiddlewareBB::Global::cache_man.at((MiddlewareBB::Global::cache_man.size() - 1 - MiddlewareBB::Global::default_ref)));
                    }
                }

                sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", MiddlewareBB::Global::ind -> length);
                write(s2, response, strlen(response));

                //Send body response
                int t;
                for (j = 0;
                    (t = write(s2, MiddlewareBB::Global::ind -> data + j, MiddlewareBB::Global::ind -> length - j)) > 0; j += t);

                if (MiddlewareBB::Global::log) {
                    printf("200 OK\n");
                }

            } else if (strcmp(path + 1, MiddlewareBB::Global::cache_man.front().path) == 0) {
                if (MiddlewareBB::Global::reference == nullptr) {
                    if (MiddlewareBB::Global::cache_man.size() > MiddlewareBB::Global::default_ref) {
                        MiddlewareBB::Global::reference = std::make_shared < MiddlewareBB::Global::file > (MiddlewareBB::Global::cache_man.at((MiddlewareBB::Global::cache_man.size() - 1 - MiddlewareBB::Global::default_ref)));
                    } else {
                        if (MiddlewareBB::Global::log) {
                            printf("404 Not Found\n");
                        }

                        sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n");
                        write(s2, response, strlen(response));
                        MiddlewareBB::Global::lock_server_sock.unlock();
                        close(s2);
                        return 0;
                    }
                }

                sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", MiddlewareBB::Global::reference -> length);
                write(s2, response, strlen(response));

                //Send body response
                int t;
                for (j = 0;
                    (t = write(s2, MiddlewareBB::Global::reference -> data + j, MiddlewareBB::Global::reference -> length - j)) > 0; j += t);

                if (MiddlewareBB::Global::log) {
                    printf("200 OK\n");
                }

            } else {
                int flag = 0;
                int n = 1;

                if (MiddlewareBB::Global::log) {
                    printf("CACHE SERVER\n");
                }

                while (!MiddlewareBB::Global::lock_cache_ts.try_lock());
                for (auto it = cache -> cbegin(); it != cache -> cend();) {

                    if (MiddlewareBB::Global::log) {
                        printf("%d:%s\n", n, it -> path);
                    }

                    if (strcmp(path + 1, it -> path) == 0) {

                        if (MiddlewareBB::Global::log) {
                            printf("200 OK\n");
                        }

                        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", it -> length);
                        write(s2, response, strlen(response));

                        //Send body response
                        int t;
                        for (j = 0;
                            (t = write(s2, it -> data + j, it -> length - j)) > 0; j += t);
                        flag = 1;
                    }
                    it++;
                    n++;
                }
                MiddlewareBB::Global::lock_cache_ts.unlock();
                if (flag) {
                    MiddlewareBB::Global::lock_server_sock.unlock();
                    continue;
                }

                if (MiddlewareBB::Global::log) {
                    printf("404 Not Found\n");
                }

                sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n");
                write(s2, response, strlen(response));

                MiddlewareBB::Global::lock_server_sock.unlock();
                close(s2);
                return 0;
            }
            MiddlewareBB::Global::lock_server_sock.unlock();
        } while (ka && recv(s2, NULL, 1, MSG_PEEK | MSG_DONTWAIT) != 0 && MiddlewareBB::Global::running);
    } catch (...) {
        MiddlewareBB::Global::lock_server_sock.unlock();
        close(s2);
        MiddlewareBB::Global::running = 0;
        throw;
    }
    return 0;
}