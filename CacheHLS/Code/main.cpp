#include <signal.h>
#include "Server.h"

struct hostent * hostnm; //hostent structure
unsigned long client_ip; //ip client
FILE * fileptr; //variables to read the config file
char file[500];

//Track signals to end program with ctrl+c
void intHandler(int dummy) {
    MiddlewareBB::Global::running = 0;
    printf("\nAll threads closed successfully\n");
}

//Entry point of program
int main(int argc, char * argv[]) {

    //Take delay from original flow
    if (argc > 1) {
        MiddlewareBB::Global::max_age = atoi(argv[1]) + 10;
    } else {
        perror("Delay is missing!");
        exit(1);
    }

    //Debugging-log mode
    if ((argc > 2) && strcmp(argv[2], "-l") == 0) {
        MiddlewareBB::Global::log = 1;
    }

    //Read config.txt file
    int len = 0; //length of the file
    fileptr = fopen("../config.txt", "r");
    while (!feof(fileptr))
        file[len++] = fgetc(fileptr);
    fclose(fileptr);
    len--;
    file[len] = 0;

    //Parsing file config.txt
    try {
        int flag = 0;
        int j = 0;

        //Skip initial "source="
        for (; file[j] != '='; j++);
        j++;

        for (int a = j; a < len; a++) {

            //Find the protocol http or https
            if (file[a] == '/' && file[a - 1] == '/' && file[a - 2] == ':') {
                file[a - 2] = 0;
                if (strcmp( & file[j], "https") == 0) {
                    MiddlewareBB::Global::https = 1;
                } else if (strcmp( & file[j], "http") == 0) {
                    MiddlewareBB::Global::https = 0;
                } else {
                    perror("Protocol not found, works only for http or https!");
                    exit(1);
                }
                flag = 1;
                a++;
                j = a;
            }

            if (flag && (file[a] == ':' || file[a] == '/')) {

                //Find client domain
                int indx = 0;
                for (int b = j; b < a; b++) {
                    MiddlewareBB::Global::client_host[indx++] = file[b];
                }
                hostnm = gethostbyname(MiddlewareBB::Global::client_host);
                if (hostnm == (struct hostent * ) 0) {
                    perror("Gethostbyname failed, domain not found\n");
                    exit(1);
                }
                client_ip = * ((unsigned long * ) hostnm -> h_addr);
                a++;
                j = a;

                //Port is specified
                if (file[a - 1] == ':') {
                    for (; file[a] != '/'; a++);
                    file[a] = 0;
                    MiddlewareBB::Global::client_port = atoi( & file[j]);
                    a++;
                    j = a;
                }

                //Port not specified, taken the default one of the protocol
                else {
                    if (MiddlewareBB::Global::https) {
                        MiddlewareBB::Global::client_port = 443;
                    } else {
                        MiddlewareBB::Global::client_port = 80;
                    }
                }
                flag = 0;
            }

            //Find the path of the file
            if (file[a - 4] == '.' && file[a - 3] == 'm' && file[a - 2] == '3' && file[a - 1] == 'u' && file[a] == '8') {
                int indx = 0;
                for (int b = j; b < a + 1; b++) {
                    MiddlewareBB::Global::client_index[indx++] = file[b];
                }
                j = a + 1;
                flag = 1;
            }
        }

        //File .m3u8 not found
        if (!flag) {
            perror("file .m3u8 not found");
            exit(1);
        }
    } catch (...) {
        perror("Parsing file config.txt fail");
        exit(1);
    }

    //Create instance of client, cache and server
    MiddlewareBB::Client * client;
    MiddlewareBB::Cache * cache;
    MiddlewareBB::Server * server;
    try {
        client = new MiddlewareBB::Client(client_ip, MiddlewareBB::Global::client_port);
        cache = new MiddlewareBB::Cache();
        server = new MiddlewareBB::Server(MiddlewareBB::Global::server_port);
    } catch (...) {
        perror("instantiation client, cache, server fail");
        exit(1);
    }

    //Set default_ref
    MiddlewareBB::Global::default_ref = atoi(argv[1]) / ((int)(MiddlewareBB::Global::wait * 2));

    //Check for stability at least one chunk
    if (MiddlewareBB::Global::default_ref < 1) {
        perror("Default reference fail");
        printf("For the stability of program delay should be of at least one chunk, that in this case is %d seconds long\n", (int)(MiddlewareBB::Global::wait * 2));
        exit(1);
    }

    //Allocate stack for child task.
    const int STACK_SIZE = 8192;
    char * stack = (char * ) malloc(STACK_SIZE * 4);
    if (!stack) {
        perror("Malloc main fail");
        exit(1);
    }

    //Clone, starts different threads
    if (clone(client -> manRequest, stack + STACK_SIZE, CLONE_VM, & MiddlewareBB::Global::ts_list) == -1) {
        MiddlewareBB::Global::running = 0;
        perror("Clone manRequest fail");
        exit(1);
    }
    if (clone(client -> tsRequest, stack + STACK_SIZE + STACK_SIZE, CLONE_VM, & MiddlewareBB::Global::ts_list) == -1) {
        MiddlewareBB::Global::running = 0;
        perror("Clone tsRequest fail");
        exit(1);
    }
    if (clone(cache -> removeExpiredFiles, stack + STACK_SIZE + STACK_SIZE + STACK_SIZE, CLONE_VM, & MiddlewareBB::Global::cache_ts) == -1) {
        MiddlewareBB::Global::running = 0;
        perror("Clone removeExpiredFiles fail");
        exit(1);
    }
    if (clone(server -> listener, stack + STACK_SIZE + STACK_SIZE + STACK_SIZE + STACK_SIZE, CLONE_VM, & MiddlewareBB::Global::cache_ts) == -1) {
        MiddlewareBB::Global::running = 0;
        perror("Clone listener fail");
        exit(1);
    }

    signal(SIGINT, intHandler);

    while (MiddlewareBB::Global::running) {
        usleep(60 * 1000000);
    }
    return 0;
}