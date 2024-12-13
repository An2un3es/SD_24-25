/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zookeeper_usage.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "server_skeleton.h"
#include "block.h"
#include "entry.h"

struct server_t *server_global;

void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    struct server_t *watcher = (struct server_t *)watcher_ctx;  // Passar o servidor como contexto

    if (type == ZOO_CHILD_EVENT && state == ZOO_CONNECTED_STATE) {
        struct String_vector children_list;  // Usar String_vector para armazenar os filhos
        int ret = zoo_get_children(wzh, "/chain", 1, &children_list);
        
        if (ret != ZOK) {
            printf("Erro ao obter filhos de /chain: %d\n", ret);
            return;  // Evitar acessar dados inválidos
        }

        sleep(1);
        // Extrair o nome do nó atual
        char *node_name = extract_node_name(watcher->znode_path);
        char **array=get_nodes_before_after(&children_list,node_name);

        if (array[0] == NULL){
            printf("ERRO AO OBTER NÓ 1\n");
        }
        printf("NO 1: %s\n",array[0]);

        if (array[1] == NULL){
            printf("ERRO AO OBTER NÓ 2\n");
        }
        printf("NO 2: %s\n",array[1]);

        int rc;

        // Ler os dados associados ao head
        char buffer[256];
        int buffer_len = sizeof(buffer);
        char tail_path[512]; // Tamanho suficiente para o caminho completo
        snprintf(tail_path, sizeof(tail_path), "/chain/%s", array[1]);

        rc = zoo_get(wzh, tail_path, 0, buffer, &buffer_len, NULL);
        if (rc != ZOK) {
            printf("Erro ao obter dados do servidor head: %s\n", tail_path);
            return ;
        }
        buffer[buffer_len] = '\0';
        array[1] = strdup(buffer);

        printf("DADOS DO SERVIDOR: %s\n", array[1]);

        // Verificar se o próximo nó mudou e atualizar a conexão com o próximo servidor
        if (server_global->next_server_name == NULL || array[1] != NULL || strcmp(array[1], server_global->next_server_name) != 0) {
            if (server_global->next_server != NULL) {
                rtable_disconnect(server_global->next_server);
            }
            sleep(3);
            server_global->next_server = rtable_connect(array[1]);
            server_global->next_server_name = strdup(array[1]);
        }

        // Liberar memória para strings duplicadas (se necessário)
        free(array[1]);
    }
}


void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {}

void create_node_chain(zhandle_t *zzh){
    char node_path[120] = "";
    strcat(node_path,"/chain");
    int new_path_len = 1024;
    char* new_path = malloc(new_path_len); 
    zoo_create(zzh, node_path, NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, new_path, new_path_len);
    free(new_path);
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

char** get_nodes_before_after(struct String_vector *strings, char* compare) {

    char **array = malloc(2 * sizeof(char *));
    if (strings == NULL || strings->count == 0 || compare == NULL) {
        array[0] = NULL;
        array[1] = NULL;
        return array;
    }

    // Ordenar a lista de nós com o casting adequado
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


struct server_t *server_init(int n_lists, char *zoo_server, char *server_ip_port) {
    if (zoo_server == NULL ) {
        printf("Erro: Argumentos inválidos para inicializar o servidor.\n");
        return NULL;
    }
    
    server_global = malloc(sizeof(struct server_t));
    if (server_global == NULL) {
        printf("Erro ao alocar memória para a estrutura server_t.\n");
        return NULL;
    }

    // Inicializar tabela
    server_global->table = server_skeleton_init(n_lists);
    if (server_global->table == NULL) {
        printf("Erro ao inicializar tabela hash.\n");
        free(server_global);
        return NULL;
    }

    // Conectar ao ZooKeeper
    printf("ENDEREÇO DO ZOOKEEPER FORNECIDO: %s\n", zoo_server);

    server_global->zh = zookeeper_init(zoo_server, my_watcher_func, 2000, 0, NULL, 0);
    if (server_global->zh == NULL) {
        printf("Erro ao conectar ao ZooKeeper.\n");
        server_skeleton_destroy(server_global->table);
        free(server_global);
        return NULL;
    }

    if (ZNONODE == zoo_exists(server_global->zh, "/chain", 0, NULL)) { 
        printf("/chain não existe! A criar nó /chain \n");
        create_node_chain(server_global->zh);
    }

    // Criar ZNode no ZooKeeper
    char *znode_path = malloc(1024);
    if (znode_path == NULL) {
        printf("Erro ao alocar memória para znode_path.\n");
        server_skeleton_destroy(server_global->table);
        zookeeper_close(server_global->zh);
        free(server_global);
        return NULL;
    }

    snprintf(znode_path, 1024, "/chain/node");
    int new_path_len = 1024;
    char *data = server_ip_port;
    
    if (zoo_create(server_global->zh, znode_path, data, strlen(data), &ZOO_OPEN_ACL_UNSAFE,
                   ZOO_EPHEMERAL | ZOO_SEQUENCE, znode_path, new_path_len) != ZOK) {
        printf("Erro ao criar ZNode no ZooKeeper.\n");
        server_skeleton_destroy(server_global->table);
        zookeeper_close(server_global->zh);
        free(znode_path);
        free(server_global);
        return NULL;
    }
    server_global->znode_path = strdup(znode_path);
    free(znode_path);

    sleep(1);

    printf("PASSA1\n");
    fflush(stdout);


    // Configurar watch na lista de filhos do nó /chain
    zoo_string *children_list = malloc(sizeof(zoo_string));
    if (children_list == NULL) {
        printf("Erro ao alocar memória para children_list.\n");
        server_skeleton_destroy(server_global->table);
        zookeeper_close(server_global->zh);
        free(server_global->znode_path);
        free(server_global);
        return NULL;
    }

    printf("PASSA2\n");
    fflush(stdout);

    if (zoo_wget_children(server_global->zh, "/chain", child_watcher, (void *)server_global, children_list) != ZOK) {  // Passar `server` como contexto
        printf("Erro ao configurar watch no nó /chain.\n");
        free(children_list);
        server_skeleton_destroy(server_global->table);
        zookeeper_close(server_global->zh);
        free(server_global->znode_path);
        free(server_global);
        return NULL;
    }

    printf("PASSA3\n");
    fflush(stdout);


    char *node_name = extract_node_name(server_global->znode_path);
    char **array= get_nodes_before_after(children_list,node_name);
    if (array == NULL) {
        printf("Erro ao alocar memória para array de nós.\n");
        free(children_list);
        server_skeleton_destroy(server_global->table);
        zookeeper_close(server_global->zh);
        free(server_global->znode_path);
        free(server_global);
        return NULL;
    }
    free(node_name);

    char buffer[512];
    int buffer_len = sizeof(buffer);

    printf("NODE ANTES: %s\n", array[0]);

    if (array[0] != NULL) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "/chain/%s", array[0]);
        if (zoo_get(server_global->zh, full_path, 0, buffer, &buffer_len, NULL) == ZOK) {
            buffer[buffer_len] = '\0'; 
            printf("Dados do ZNode: %s\n", buffer);
        } else {
            printf("Erro ao obter dados do ZNode: %s\n", array[0]);
            free(array);
            free(children_list);
            return NULL;
        }

        // Sincronizar com antecessor
        struct rtable_t *prev_server = rtable_connect(buffer);
        if (prev_server != NULL) {
            struct entry_t **entrys = rtable_get_table(prev_server);
            if (entrys != NULL) {
                for (int i = 0; entrys[i] != NULL; i++) {
                    table_put(server_global->table, entrys[i]->key, entrys[i]->value); 
                }
            }
            rtable_free_entries(entrys);
            rtable_disconnect(prev_server);
        }
    }

    if (array[1] != NULL) {
        char full_path2[512];
        snprintf(full_path2, sizeof(full_path2), "/chain/%s", array[1]);
        if (zoo_get(server_global->zh, full_path2, 0, buffer, &buffer_len, NULL) == ZOK) {
            buffer[buffer_len] = '\0'; 
            server_global->next_server = rtable_connect(buffer); 
            server_global->next_server_name = strdup(buffer); 
        } else {
            printf("Erro ao obter dados do ZNode: %s\n", array[1]);
            free(array);
            free(children_list);
            return NULL;
        }
    } else {
        server_global->next_server = NULL;
        server_global->next_server_name = NULL; 
    }

    // Liberar memória
    free(array);
    free(children_list);

    printf("Server inicializado com sucesso.\n");
    return server_global;
}


int rtable_put_next(char *key, struct block_t *value){

    if(server_global->next_server!=NULL){
        if (value == NULL || key == NULL)
            return -1;

        // Cria uma cópia da chave
        char *key_copy = strdup(key);
        if (key_copy == NULL) {
            return -1; 
        }

        // Cria uma cópia do bloco usando block_duplicate
        struct block_t *value_copy = block_duplicate(value);
        if (value_copy == NULL) {
            free(key_copy); 
            return -1; 
        }

        // Cria a nova entry
        struct entry_t *entry = entry_create(key_copy, value_copy);
        rtable_put(server_global->next_server,entry);
        printf("CHEGOU NO NEXT.\n");
    }else{
        printf("CHEGOU NA TAIL.\n");
    }
    return 0;

}

int rtable_del_next(char *key){

    if(server_global->next_server!=NULL){

        if (key == NULL)
            return -1;

        // Cria uma cópia da chave
        char *key_copy = strdup(key);
        if (key_copy == NULL) {
            return -1; 
        }

        rtable_del(server_global->next_server,key_copy);
        printf("CHEGOU NO NEXT.\n");
    }else{
        printf("CHEGOU NA TAIL.\n");
    }
    return 0;

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
