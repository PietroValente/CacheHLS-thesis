#include "Global.h"

// Define the variables
std::queue < std::string > MiddlewareBB::Global::ts_list;
std::vector < MiddlewareBB::Global::file > MiddlewareBB::Global::cache_ts;
std::vector < MiddlewareBB::Global::file > MiddlewareBB::Global::cache_man;
std::shared_ptr < MiddlewareBB::Global::file > MiddlewareBB::Global::ind;
std::shared_ptr < MiddlewareBB::Global::file > MiddlewareBB::Global::reference = nullptr;

double MiddlewareBB::Global::wait = 1.0;

int MiddlewareBB::Global::client_sock = 0;
int MiddlewareBB::Global::client_port = -1;
int MiddlewareBB::Global::server_port = 8080;
int MiddlewareBB::Global::server_sock = 0;
int MiddlewareBB::Global::max_age = 0;
int MiddlewareBB::Global::default_ref = 2;

//FLAGS
int MiddlewareBB::Global::running = 1;
int MiddlewareBB::Global::https = 1;
int MiddlewareBB::Global::log = 0;

SSL_CTX * MiddlewareBB::Global::ctx;
SSL * MiddlewareBB::Global::ssl;

char MiddlewareBB::Global::client_index[100];
char MiddlewareBB::Global::client_host[100];

std::mutex MiddlewareBB::Global::lock_ts_list;
std::mutex MiddlewareBB::Global::lock_cache_ts;
std::mutex MiddlewareBB::Global::lock_cache_man;
std::mutex MiddlewareBB::Global::lock_client_sock;
std::mutex MiddlewareBB::Global::lock_server_sock;