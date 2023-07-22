#include "Cache.h"

//Class that create a connection and manage request with client, that in our case is ServerHLS

namespace MiddlewareBB {
    class Client {
        public: 
        Client(unsigned long targetip, int port); //constructor
        static MiddlewareBB::Global::file getFile(char * path); //setting the path returns the struct file from the ServerHLS
        static int manRequest(void * list); //thread request manifest and add the last block (.ts) if it's not already requested

        private: 
        inline static struct header { //struct for response headers
            char * name;
            char * value;
        }
        h[100];

        static char * entity; //save file data before return
        static char * statusline; //status line of response
        static char * first_ts; //first ts of manifest
        static char request[5000], response[5000], manifest[100], manifest_path[100], ts[100], last_chunk[100], chunk_time[100];
        static int entity_length, h_num, c_read, tmp, j;
        static struct sockaddr_in addr;
    };
};