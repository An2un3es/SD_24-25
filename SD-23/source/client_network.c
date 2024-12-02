/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "client_stub.h"
#include "client_stub-private.h"
#include "htmessages.pb-c.h"
#include "message-private.h"  
#include <zookeeper/zookeeper.h>

// Estrutura para armazenar o contexto do watcher
typedef struct {
    struct rtable_t *head;
    struct rtable_t *tail;
} watcher_context_t;

// Variável global para a conexão ao ZooKeeper
zhandle_t *zookeeper_handle = NULL;

/* Função de callback para eventos do ZooKeeper */
void zookeeper_watch_children(zhandle_t *zh, int type, int state, const char *path, void *watcher_ctx) {
    if (type == ZOO_CHILD_EVENT) {
        printf("Atualizando conexões com head e tail.\n");
        watcher_context_t *context = (watcher_context_t *)watcher_ctx;

        // Atualizar head e tail
        if (update_head_and_tail(context->head, context->tail) < 0) {
            fprintf(stderr, "Erro ao atualizar conexões com head e tail.\n");
        }
    }

    // Re-registrar o watch
    zoo_wget_children(zookeeper_handle, "/chain", zookeeper_watch_children, watcher_ctx, NULL);
}

/* Conexão ao ZooKeeper */
int zookeeper_connect(const char *zookeeper_address, watcher_context_t *context) {
    zookeeper_handle = zookeeper_init(zookeeper_address, NULL, 30000, 0, 0, 0);
    if (zookeeper_handle == NULL) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper.\n");
        return -1;
    }

    // Registrar watcher inicial
    if (zoo_wget_children(zookeeper_handle, "/chain", zookeeper_watch_children, context, NULL) != ZOK) {
        fprintf(stderr, "Erro ao registrar watcher em /chain.\n");
        zookeeper_close(zookeeper_handle);
        return -1;
    }

    return 0;
}

/* Atualiza as conexões com os servidores head e tail */
int update_head_and_tail(struct rtable_t *head, struct rtable_t *tail) {
    struct String_vector children;
    if (zoo_get_children(zookeeper_handle, "/chain", 1, &children) != ZOK) {
        fprintf(stderr, "Erro ao obter filhos de /chain.\n");
        return -1;
    }

    char *head_server = NULL;
    char *tail_server = NULL;

    // Identificar head e tail com base nos IDs dos servidores
    for (int i = 0; i < children.count; i++) {
        if (head_server == NULL || strcmp(children.data[i], head_server) < 0) {
            head_server = children.data[i];
        }
        if (tail_server == NULL || strcmp(children.data[i], tail_server) > 0) {
            tail_server = children.data[i];
        }
    }

    if (!head_server || !tail_server) {
        fprintf(stderr, "Erro: Não foi possível identificar head e tail.\n");
        deallocate_String_vector(&children);
        return -1;
    }

    printf("Novo head: %s, Novo tail: %s\n", head_server, tail_server);

    // Atualizar conexões
    network_close(head);
    free(head->server_address);
    head->server_address = strdup(head_server);
    if (network_connect(head) < 0) {
        fprintf(stderr, "Erro ao conectar ao novo head.\n");
        deallocate_String_vector(&children);
        return -1;
    }

    network_close(tail);
    free(tail->server_address);
    tail->server_address = strdup(tail_server);
    if (network_connect(tail) < 0) {
        fprintf(stderr, "Erro ao conectar ao novo tail.\n");
        deallocate_String_vector(&children);
        return -1;
    }

    deallocate_String_vector(&children);
    return 0;
}

// Função auxiliar para comparar identificadores
int compare_identifiers(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* Função que obtém o endereço do servidor head e do servidor tail, para o cliente se
* conectar com os mesmos.
* Retorna 0 em caso de sucesso e -1 em caso de erro.
*/
int get_head_and_tail_addresses(char **head_address, char **tail_address) {
    struct String_vector children;
    
    // Obter a lista de filhos do zNode /chain
    int rc = zoo_get_children(zookeeper_handle, "/chain", 0, &children);
    if (rc != ZOK) {
        fprintf(stderr, "Erro ao obter filhos do zNode /chain: %d\n", rc);
        return -1;
    }

    if (children.count == 0) {
        fprintf(stderr, "Nenhum servidor ligado em /chain.\n");
        return -1;
    }

    // Ordenar os filhos para determinar head e tail.
    qsort(children.data, children.count, sizeof(char *), compare_identifiers);

    // Alocar memória para head e tail
    char head_path[256], tail_path[256];
    snprintf(head_path, sizeof(head_path), "/chain/%s", children.data[0]);
    snprintf(tail_path, sizeof(tail_path), "/chain/%s", children.data[children.count - 1]);

    // Ler os dados associados ao head
    char buffer[256];
    int buffer_len = sizeof(buffer);
    rc = zoo_get(zookeeper_handle, head_path, 0, buffer, &buffer_len, NULL);
    if (rc != ZOK) {
        fprintf(stderr, "Erro ao obter dados do servidor head: %s\n", head_path);
        deallocate_String_vector(&children);
        return -1;
    }
    buffer[buffer_len] = '\0';
    *head_address = strdup(buffer);

    // Ler os dados associados ao tail
    buffer_len = sizeof(buffer);
    rc = zoo_get(zookeeper_handle, tail_path, 0, buffer, &buffer_len, NULL);
    if (rc != ZOK) {
        fprintf(stderr, "Erro ao obter dados do servidor tail: %s\n", tail_path);
        free(*head_address);
        deallocate_String_vector(&children);
        return -1;
    }
    buffer[buffer_len] = '\0';
    *tail_address = strdup(buffer);

    // Libertar a memória dos filhos
    deallocate_String_vector(&children);

    return 0;
}



/* Esta função deve:
* - Obter o endereço do servidor (struct sockaddr_in) com base na
* informação guardada na estrutura rtable;
* - Estabelecer a ligação com o servidor;
* - Guardar toda a informação necessária (e.g., descritor do socket)
* na estrutura rtable;
* - Retornar 0 (OK) ou -1 (erro).
*/
int network_connect(struct rtable_t *rtable){

    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Error creating TCP socket");
      return -1;
    }

    rtable->sockfd=sockfd;

    // Preenche a estrutura server
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);


    // Converte o endereço IP
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(rtable-> sockfd);
        return -1;
    }

    // Estabelece a conexão
    if (connect(rtable-> sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(rtable-> sockfd);
        return -1;
    }

    return 0;

}

/* Esta função deve:
* - Obter o descritor da ligação (socket) da estrutura rtable_t;
* - Serializar a mensagem contida em msg;
* - Enviar a mensagem serializada para o servidor;
* - Esperar a resposta do servidor;
* - De-serializar a mensagem de resposta;   
* - Tratar de forma apropriada erros de comunicação;
* - Retornar a mensagem de-serializada ou NULL em caso de erro.
*/
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg){

    if (rtable == NULL || msg == NULL) {
        fprintf(stderr, "Erro: %s é NULL.\n", rtable == NULL ? "rtable" : "msg");
        return NULL;
    }

    // Obter o descritor da ligação (socket) da estrutura rtable_t
    int socketfd = rtable->sockfd;
    if (socketfd <= 0) {
        fprintf(stderr, "Erro: descritor de socket inválido.\n");
        return NULL;
    }


    // Serializar a mensagem contida em msg
    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *msg_serialized = (uint8_t *)malloc(msg_size);
    if (msg_serialized == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória para a mensagem serializada.\n");
        return NULL;
    }
    message_t__pack(msg, msg_serialized);


    // Enviar o tamanho da mensagem ao servidor
    uint32_t msg_size_network = htonl(msg_size);  // Converter para ordem de rede
    if (write_all(socketfd, &msg_size_network, sizeof(msg_size_network)) < 0) {
        fprintf(stderr, "Erro ao enviar o tamanho da mensagem ao servidor.\n");
        free(msg_serialized);
        return NULL;
    }


    // Enviar a mensagem serializada para o servidor
    if (write_all(socketfd, msg_serialized, msg_size) < 0) {
        fprintf(stderr, "Erro ao enviar a mensagem ao servidor.\n");
        free(msg_serialized);
        return NULL;
    }
    free(msg_serialized);  


    // Esperar a resposta do servidor (tamanho da mensagem)
    uint32_t response_size_network;
    if (read_all(socketfd, &response_size_network, sizeof(response_size_network)) < 0) {
        fprintf(stderr, "Erro ao receber o tamanho da resposta do servidor.\n");
        return NULL;
    }
    size_t response_size = ntohl(response_size_network);  // Converter size


    // Alocar memória para a resposta e receber a mensagem completa
    uint8_t *response_buf = (uint8_t *)malloc(response_size);
    if (response_buf == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória para a resposta.\n");
        return NULL;
    }
    if (read_all(socketfd, response_buf, response_size) < 0) {
        fprintf(stderr, "Erro ao receber a resposta do servidor.\n");
        free(response_buf);
        return NULL;
    }

    // De-serializar a mensagem de resposta
    MessageT *response_msg = message_t__unpack(NULL, response_size, response_buf);
    free(response_buf);  // Libertar a memória do buffer da resposta

    if (response_msg == NULL) {
        fprintf(stderr, "Erro ao deserializar a resposta do servidor.\n");
        return NULL;
    }

    return response_msg;

}


/* Fecha a ligação estabelecida por network_connect().
* Retorna 0 (OK) ou -1 (erro).
*/
int network_close(struct rtable_t *rtable) {
    if (rtable && rtable->sockfd > 0) {
        close(rtable->sockfd);
    }
    return 0;
}