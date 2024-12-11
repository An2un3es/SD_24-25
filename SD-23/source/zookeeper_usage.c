#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zookeeper_usage.h"
#include "client_stub.h"
#include "server_skeleton.h"
struct server_t * server_g;

void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    if (type == ZOO_CHILD_EVENT && state == ZOO_CONNECTED_STATE) {
        zoo_string *children_list = malloc(sizeof(zoo_string));
        if (ZOK == zoo_wget_children(wzh, "/chain", child_watcher, watcher_ctx, children_list)) {
            char **array = malloc(2 * sizeof(char *));
            char *node_name = extract_node_name(server_g->znode_path);
            get_nodes_before_after(children_list, array, node_name);

            if (array[1] != NULL && strcmp(array[1], server_g->next_server_name) != 0) {
                // Atualize a conexão com o próximo servidor
                if (server_g->next_server != NULL) {
                    rtable_disconnect(server_g->next_server);
                }
                server_g->next_server = rtable_connect(array[1]);
                server_g->next_server_name = strdup(array[1]);
            }
            free(array);
        }
        free(children_list);
    }
}

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


char** get_nodes_before_after(struct String_vector *strings, char** array, char* compare){

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
        return NULL;
    }

    // Nó anterior
    array[0] = (position > 0) ? strdup(strings->data[position - 1]) : NULL;
    // Nó seguinte
    array[1] = (position < strings->count - 1) ? strdup(strings->data[position + 1]) : NULL;
    return array;
}

struct server_t *server_init(struct server_t *servidor,int n_lists, char *zoo_server, char *server_ip_port) {
    if (zoo_server == NULL ) {
        fprintf(stderr, "Erro: Argumentos inválidos para inicializar o servidor.\n");
        return NULL;
    }

    server_g=servidor;
    if (server_g == NULL) {
        fprintf(stderr, "Erro ao alocar memória para a estrutura server_t.\n");
        return NULL;
    }

    // Inicializar tabela
    server_g->table = server_skeleton_init(n_lists);
    if (server_g->table == NULL) {
        fprintf(stderr, "Erro ao inicializar tabela hash.\n");
        free(server_g);
        return NULL;
    }

    // Conectar ao ZooKeeper
    printf("ENDEREÇO DO ZOOKEEPER FORNECIDO: %s\n", zoo_server);

    server_g->zh = zookeeper_init(zoo_server, my_watcher_func, 2000, 0, NULL, 0);
    if (server_g->zh == NULL) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper.\n");
        server_skeleton_destroy(server_g->table);
        free(server_g);
        return NULL;
    }

    if (ZNONODE == zoo_exists(servidor->zh, "/chain", 0, NULL)) { //confirmar
                fprintf(stderr, "/chain não existe! \
                    A criar nó /chain \n");

        create_node_chain(servidor->zh);
    }

    // Criar ZNode no ZooKeeper
    char *znode_path = malloc(1024);
    if (znode_path == NULL) {
        fprintf(stderr, "Erro ao alocar memória para znode_path.\n");
        server_skeleton_destroy(server_g->table);
        zookeeper_close(server_g->zh);
        free(server_g);
        return NULL;
    }

    snprintf(znode_path, 1024, "/chain/node");
    int new_path_len = 1024;
    char *data = server_ip_port;
    if (zoo_create(server_g->zh, znode_path, data, strlen(data), &ZOO_OPEN_ACL_UNSAFE,
                   ZOO_EPHEMERAL | ZOO_SEQUENCE, znode_path, new_path_len) != ZOK) {
        fprintf(stderr, "Erro ao criar ZNode no ZooKeeper.\n");
        server_skeleton_destroy(server_g->table);
        zookeeper_close(server_g->zh);
        free(znode_path);
        free(server_g);
        return NULL;
    }
    server_g->znode_path = strdup(znode_path);
    free(znode_path);

    // Configurar watch na lista de filhos do nó /chain
    zoo_string *children_list = malloc(sizeof(zoo_string));
    if (children_list == NULL) {
        fprintf(stderr, "Erro ao alocar memória para children_list.\n");
        server_skeleton_destroy(server_g->table);
        zookeeper_close(server_g->zh);
        free(server_g->znode_path);
        free(server_g);
        return NULL;
    }

    if (zoo_wget_children(server_g->zh, "/chain", child_watcher, NULL, children_list) != ZOK) {
        fprintf(stderr, "Erro ao configurar watch no nó /chain.\n");
        free(children_list);
        server_skeleton_destroy(server_g->table);
        zookeeper_close(server_g->zh);
        free(server_g->znode_path);
        free(server_g);
        return NULL;
    }

    // Identificar antecessor e sincronizar tabela
    char **array = malloc(2 * sizeof(char *));
    if (array == NULL) {
        fprintf(stderr, "Erro ao alocar memória para array de nós.\n");
        free(children_list);
        server_skeleton_destroy(server_g->table);
        zookeeper_close(server_g->zh);
        free(server_g->znode_path);
        free(server_g);
        return NULL;
    }

    char *node_name = extract_node_name(server_g->znode_path);
    get_nodes_before_after(children_list, array, node_name);
    free(node_name);

    char buffer[512];
    int buffer_len = sizeof(buffer);

    if (array[0] != NULL) {
        if (zoo_get(server_g->zh, array[0], 0, buffer, &buffer_len, NULL) == ZOK) {
            buffer[buffer_len] = '\0'; 
            printf("Dados do ZNode: %s\n", buffer);
        } else {
            printf("Erro ao obter dados do ZNode: %s\n", array[1]);
        }
        // Sincronizar com antecessor
        struct rtable_t *prev_server = rtable_connect(buffer);
        if (prev_server != NULL) {
            struct entry_t **entrys = rtable_get_table(prev_server);
            if (entrys != NULL) {
                for (int i = 0; entrys[i] != NULL; i++) {
                    table_put(server_g->table,entrys[i]->key,entrys[i]->value);  //TODO @Rodrigo cofirmar
                }   
            }
            rtable_free_entries(entrys);
            rtable_disconnect(prev_server);
        }
    }
    if (array[1] != NULL) {
        if (zoo_get(server_g->zh, array[1], 0, buffer, &buffer_len, NULL) == ZOK) {
            buffer[buffer_len] = '\0'; 
            server_g->next_server = rtable_connect(buffer); 
            server_g->next_server_name = strdup(buffer); 
        }
    }else{
        server_g->next_server = NULL;
        server_g->next_server_name = NULL; 
    }

    free(array);
    free(children_list);


    printf("Server inicializado com sucesso.\n");
    return server_g;
}

void server_destroy(struct server_t *server) {
    if (server == NULL) return;

    // Fechar conexão com o próximo servidor
    if (server->next_server != NULL) {
        rtable_disconnect(server->next_server);
    }

    // Fechar conexão com o ZooKeeper
    if (server->zh != NULL) {
        zookeeper_close(server->zh);
    }

    // Liberar tabela
    if (server->table != NULL) {
        server_skeleton_destroy(server->table);
    }

    // Liberar strings
    if (server->znode_path != NULL) {
        free(server->znode_path);
    }
    if (server->next_server_name != NULL) {
        free(server->next_server_name);
    }

    free(server);
}

