#include "Cache.h"

//Class that manages cache

//Thread that remove expired files from cache
int MiddlewareBB::Cache::removeExpiredFiles(void * c) {
    try {
        std::vector < MiddlewareBB::Global::file > * cache = (std::vector < MiddlewareBB::Global::file > * ) c;
        int flag = 1;
        int n = -1;

        while (MiddlewareBB::Global::running) {
            usleep(MiddlewareBB::Global::wait * 1000000);

            int server_ready = MiddlewareBB::Global::cache_man.size() - 1 - MiddlewareBB::Global::default_ref;

            //Countdown
            if (flag && (abs(server_ready - 1) != n)) {
                printf("%d\n", abs(server_ready - 1));
                n = abs(server_ready - 1);
            }

            //Server ready
            if (flag && (server_ready > 0)) {
                printf("***********************\nSERVER READY FOR A DELAY OF %d SECONDS\n***********************\n", (int)(MiddlewareBB::Global::default_ref * MiddlewareBB::Global::wait * 2));
                flag = 0;
                printf("Output address: http://localhost:%d/%s\n", MiddlewareBB::Global::server_port, MiddlewareBB::Global::ind -> path);
            }

            //Scroll cache
            while (!MiddlewareBB::Global::lock_cache_man.try_lock());
            for (auto it = cache -> cbegin(); it != cache -> cend();) {
                //Check to be sure no manifest remain in cache
                auto age = time(nullptr) - it -> arrive_at;
                if (age > MiddlewareBB::Global::max_age) {
                    it = cache -> erase(it);
                    MiddlewareBB::Cache::printCache();
                    continue;
                } else {
                    it++;
                }
            }
            MiddlewareBB::Global::lock_cache_man.unlock();
        }
        MiddlewareBB::Global::running = 0;
    } catch (...) {
        MiddlewareBB::Global::running = 0;
        throw;
    }
    return 0;
}

//Prints all path in ts_cache
void MiddlewareBB::Cache::printCache() {
    std::vector < MiddlewareBB::Global::file > * c = & MiddlewareBB::Global::cache_man;

    if (MiddlewareBB::Global::log) {
        printf("MANIFEST CACHE\n");
        for (auto it = c -> cbegin(); it != c -> cend();) {
            printf("%s\n", it -> first_ts);
            ++it;
        }
        printf("\n");
    }
}