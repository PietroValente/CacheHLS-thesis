#include "Global.h"

//Class that manages cache

namespace MiddlewareBB {
    class Cache {
        public: 
        static int removeExpiredFiles(void * c); //thread that remove expired files from cache
        static void printCache(); //prints all path in ts_cache
    };
};