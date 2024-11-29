#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zookeeper_usage.h"


void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {}


void create_node_chain(zhandle_t *zzh){

    char node_path[120] = "";
    strcat(node_path,"/chain");
    int new_path_len = 1024;
    char* new_path = malloc (new_path_len); 
    zoo_create(zzh, node_path, NULL, 0, & ZOO_OPEN_ACL_UNSAFE, 0, new_path, new_path_len);
    free (new_path);
}

char *extract_node_name(char *path) {
    if (path == NULL) {
        return NULL;
    }

    const char *last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return NULL;
    }

    // Retornar o nome do nó (após a último /)
    return strdup(last_slash + 1); 
}


void get_nodes_before_after(struct String_vector *strings, char** array, char* compare){

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
}
