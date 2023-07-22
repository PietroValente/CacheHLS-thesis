#include "Client.h"

namespace MiddlewareBB {
    class Server {
        public: 
        Server(int port); //constructor
        static int listener(void * ca); //thread that accepts connections and creates a new thread for each new one
        static int requestManager(void * ca);

        private: 
        inline static struct header { //struct for response headers
            char * n;
            char * v;
        }
        h[100];

        static int c, i, j, k, s, s2, ka, ref;
        static socklen_t len;
        inline static int yes = 1;

        static char * commandline, * method, * path, * ver;
        static char request[5000], request2[5000], response[5000], command[100];
        static struct sockaddr_in addr, remote_addr;
    };
};