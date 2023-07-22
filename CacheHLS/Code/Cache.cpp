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
            for (auto it = cache -> cbegin(); it != cache -> cend();) {
                auto age = time(nullptr) - it -> arrive_at;

                if (age > MiddlewareBB::Global::max_age) {

                    //Delete ts from the cache
                    while (!MiddlewareBB::Global::lock_cache_ts.try_lock());
                    it = cache -> erase(it);
                    printCache();
                    MiddlewareBB::Global::lock_cache_ts.unlock();
                } else {
                    ++it;
                }
            }

            for (auto it2 = MiddlewareBB::Global::cache_man.cbegin(); it2 != MiddlewareBB::Global::cache_man.cend();) {

                //Delete man from the cache
                auto age2 = time(nullptr) - it2 -> arrive_at;
                if (age2 > MiddlewareBB::Global::max_age) {
                    while (!MiddlewareBB::Global::lock_cache_man.try_lock());
                    it2 = MiddlewareBB::Global::cache_man.erase(it2);
                    MiddlewareBB::Global::lock_cache_man.unlock();
                } else {
                    ++it2;
                }
            }
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
    std::vector < MiddlewareBB::Global::file > * c = & MiddlewareBB::Global::cache_ts;
    std::vector < MiddlewareBB::Global::file > * cm = & MiddlewareBB::Global::cache_man;

    if (MiddlewareBB::Global::log) {

        printf("CHUNK CACHE\n");
        int n = 1;
        for (auto it = c -> cbegin(); it != c -> cend();) {
            printf("%d:%s\n", n, it -> path);
            ++it;
            ++n;
        }
        printf("\n");

        printf("MANIFEST CACHE\n");
        n = 1;
        for (auto it = cm -> cbegin(); it != cm -> cend();) {
            printf("%d:%s\n", n, it -> path);
            ++it;
            ++n;
        }
        printf("\n");

    }
}