#ifndef _STATS_PRIVATE_H
#define _STATS_PRIVATE_H /* Módulo STATS_PRIVATE_H */

#include <pthread.h>
#include <stdint.h>

typedef struct{

    uint32_t total_operations;      
    uint64_t total_time;
    uint32_t connected_clients;
    pthread_mutex_t stats_mutex;
}statistics_t;

//extern para ser acedida pelos outros módulos
extern statistics_t server_stats;

#endif