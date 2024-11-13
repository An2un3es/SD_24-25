#ifndef _STATS__H
#define _STATS__H /* Módulo STATS__H */

#include <pthread.h>

typedef struct{

    uint32_t total_operations;      
    uint64_t total_time;
    uint32_t connected_clients;
    pthread_mutex_t stats_mutex;
}statistics_t;

struct statistics_t {
    uint32_t total_operations;
    uint64_t total_time;
    uint32_t connected_clients;
};

//extern para ser acedida pelos outros módulos
extern statistics_t server_stats;

#endif