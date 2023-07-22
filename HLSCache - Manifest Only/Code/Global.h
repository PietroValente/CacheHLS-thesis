#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string.h>

//Class that holds all global variables shared by threads

namespace MiddlewareBB {
    class Global {
        public: 
        struct file {
            char * path; //path with name at the end, example: out/u/bbb/qxa/manifest_3.m3u8
            char * data; //file data
            char * first_ts; //first ts in the manifest file
            int length; //length of file
            long arrive_at; //arrival time
        };
        static std::vector < file > cache_man; //cache with man files
        static std::shared_ptr < file > ind; //pointer to index file
        static std::shared_ptr < file > reference; //pointer to reference file of first ts of manifest

        static double wait; //time to wait before ask a manifest, default 1 but will be (chunk duration/2)
    
        static int client_sock; //sock of client
        static int client_port; //port of client
        static int server_port; //port of server
        static int server_sock; //sock of server
        static int max_age; //max_age for expiration files in cache_ts
        static int default_ref; //default delay on manifest reference

        //FLAGS
        static int running; //if a thread has an exception, running=0 and all threads stop
        static int https; //flag for https, default is true
        static int log; //debugger log version

        static SSL_CTX *ctx; //ssl contex
        static SSL *ssl; //ssl

        static char client_index[100]; //path of client index
        static char client_host[100]; //host of client

        //locks for shared variables
        static std::mutex lock_cache_man; //operation on cache_man
        static std::mutex lock_client_sock; //operation on receiving socket (client side)
        static std::mutex lock_server_sock; //operation on sending socket (server side)
    };
};