#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zookeeper_usage.h"

void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {}


char ** get_nodes_before_after(struct String_vector *strings, char** array, char* compare){

    if (strings == NULL || strings->count == 0 || compare == NULL) {
        array[0] = NULL;
        array[1] = NULL;
        return array;
    }

    // Ordenar a lista de nós para garantir a sequência correta
    qsort(strings->data, strings->count, sizeof(char *), (int (*)(const void *, const void *))strcmp);

    int position = -1;

    // Encontrar a posição do nó `compare`
    for (int i = 0; i < strings->count; i++) {
        if (strcmp(strings->data[i], compare) == 0) {
            position = i;
            break;
        }
    }

    if (position == -1) { // Nó `compare` não encontrado
        array[0] = NULL;
        array[1] = NULL;
        return array;
    }

    // Nó anterior
    array[0] = (position > 0) ? strdup(strings->data[position - 1]) : NULL;
    // Nó seguinte
    array[1] = (position < strings->count - 1) ? strdup(strings->data[position + 1]) : NULL;

    return array;
}
