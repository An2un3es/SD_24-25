#ifndef _STATS__H
#define _STATS__H /* Módulo STATS__H */

#include <pthread.h>
#include <stdint.h>

struct statistics_t {
    uint32_t total_operations;
    uint64_t total_time;
    uint32_t connected_clients;
};

#endif