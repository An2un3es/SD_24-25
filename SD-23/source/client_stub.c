/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "block.h"
#include "entry.h"
#include "client_stub-private.h"
#include "client_network.h"
#include "htmessages.pb-c.h"
#include "stats.h"
#include <zookeeper/zookeeper.h>

// Estrutura para armazenar o contexto do watcher
typedef struct {
    struct rtable_t *head;
    struct rtable_t *tail;
} watcher_context_t;

// Variável global para a conexão ao ZooKeeper
zhandle_t *zookeeper_handle = NULL;

int get_head_and_tail_addresses(char **head_address, char **tail_address);

// Atualiza as conexões com os servidores head e tail
int update_head_and_tail(struct rtable_t *head, struct rtable_t *tail) {
    struct String_vector children;
    memset(&children, 0, sizeof(children));

    // Obter filhos de /chain de forma síncrona
    int rc = zoo_get_children(zookeeper_handle, "/chain", 1, &children);
    if (rc != ZOK) {
        fprintf(stderr, "Erro ao obter filhos de /chain: %d\n", rc);
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
    rtable_disconnect(head);
    if(get_head_and_tail_addresses(&head_server, &tail_server )<0){
  
        return -1;
    }
  
    head = rtable_connect(head_server);
    if (head ==NULL) {
        fprintf(stderr, "Erro ao conectar ao novo head.\n");
        deallocate_String_vector(&children);
        return -1;
    }
    rtable_disconnect(tail);
    if(strcmp(head_server,tail_server) == 0){
        tail ->server_address = strdup(head ->server_address);
        tail ->server_port= head->server_port;
        tail ->sockfd= head->sockfd;
    }else{
        tail = rtable_connect(tail_server);
        if (tail == NULL) {
            fprintf(stderr, "Erro ao conectar ao novo head.\n");
            deallocate_String_vector(&children);
            return -1;
        }
    }
        deallocate_String_vector(&children);
        return 0;
}
// Conexão ao ZooKeeper
int zookeeper_connect(const char *zookeeper_address, watcher_context_t *context) {
    zookeeper_handle = zookeeper_init(zookeeper_address, NULL, 30000, 0, 0, 0);
    if (zookeeper_handle == NULL) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper.\n");
        return -1;
    }

    // Registrar watcher inicial e atualizar head e tail
    if (update_head_and_tail(context->head, context->tail) < 0) {
        fprintf(stderr, "Erro ao registrar watcher inicial e atualizar head e tail.\n");
        zookeeper_close(zookeeper_handle);
        return -1;
    }

    return 0;
}

// Função auxiliar para comparar identificadores
int compare_identifiers(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Obter endereços do head e tail
int get_head_and_tail_addresses(char **head_address, char **tail_address) {
    struct String_vector children;
    int rc;

    // Obter a lista de filhos do zNode /chain
    rc = zoo_get_children(zookeeper_handle, "/chain", 0, &children);
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
    snprintf(head_path, sizeof(head_path), "/chain/%s", children.data[0]); // Menor ID -> head
    snprintf(tail_path, sizeof(tail_path), "/chain/%s", children.data[children.count - 1]); // Maior ID -> tail

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

    return 0; // Sucesso
}

































/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port) {

    struct rtable_t *rtable = (struct rtable_t *)malloc(sizeof(struct rtable_t));

    if (rtable == NULL) {
        return NULL;
    }

    char *copy = strdup(address_port);
    if (copy == NULL) {
        free(rtable);
        return NULL;
    }

    char *host = strtok(copy, ":");
    char *port_str = strtok(NULL, ":");
    host = strcmp(host,"localhost") == 0? "127.0.0.1": host;

    printf("IP: %s\n", host);
    printf("PORTO: %s\n", port_str);

    // Verifica se host e port_str não são NULL
    if (host == NULL || port_str == NULL) {
        printf( "Erro: Formato esperado é <ip>:<port>\n");
        free(copy);
        free(rtable);
        return NULL;
    }

    // Copia o endereço do servidor
    rtable->server_address = strdup(host);
    rtable->server_port = atoi(port_str);

    // Limpa a memória de 'copy' pois já foi duplicada em rtable->server_address
    free(copy);

    // Tenta conectar ao servidor
    if (network_connect(rtable) < 0) {
        free(rtable->server_address);
        free(rtable);
        return NULL;
    }
    return rtable;
}


/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem, ou -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable)
{
    if (rtable == NULL || rtable->sockfd < 0)
        return -1;

    if (network_close(rtable) == -1)
    {
        printf("Erro ao disconectar ao servidor");
        return -1;
    }
    // Resetar o descritor do socket na rtable
    rtable->sockfd = -1; 
    free(rtable->server_address);
    free(rtable);
    return 0;
}

/* Função para adicionar uma entrada na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Retorna 0 (OK, em adição/substituição), ou -1 (erro).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry){

    // Criar a mensagem a enviar
    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type=MESSAGE_T__C_TYPE__CT_ENTRY;

    // Preencher a estrutura EntryT dentro da mensagem
    EntryT msg_entry;
    entry_t__init(&msg_entry);
    msg_entry.key = entry->key;
    msg_entry.value.len = entry->value->datasize;
    msg_entry.value.data = (uint8_t *) entry->value->data;

    // Atribuir a EntryT à mensagem
    msg.entry = &msg_entry;

    // Enviar a mensagem e receber a resposta
    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem");
        return -1;  
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return -1;
    }

    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_PUT + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return -1;
    }

    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);
    free(entry->value);
    free(entry);
    return 0;
}

/* Retorna o bloco da entrada da tabela com chave key, ou NULL caso não exista
* ou se ocorrer algum erro.
*/
struct block_t *rtable_get(struct rtable_t *rtable, char *key){

    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_GET;
    msg.c_type=MESSAGE_T__C_TYPE__CT_KEY;

    msg.key = key;

    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem");
        return NULL;  
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }


    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_GET+1 || response->c_type != MESSAGE_T__C_TYPE__CT_VALUE) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Alocar memória para os dados antes de block_create
    void *data = malloc(response->value.len);
    if (data == NULL) {
        perror("Erro ao alocar memória para os dados");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Criar o block_t
    struct block_t *result_block = block_create(response->value.len, data);
    if (result_block == NULL) {
        perror("Erro ao criar block_t");
        block_destroy(result_block);
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Copiar os dados recebidos para o block_t
    memcpy(result_block->data, response->value.data, response->value.len);

    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);

    return result_block;

}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key){

    if (key == NULL) {
        printf("Chave inválida\n");
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type=MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem");
        return -1;
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return -1;
    }

    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_DEL + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return -1;
    }

    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);

    return 0;

}

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable){

    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type=MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem");
        return -1;
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return -1;
    }

    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_SIZE + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_RESULT) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return -1;
    }

    int size = response->result;

    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);

    return size;

}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable){ 

    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type=MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem");
        return NULL;
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_GETKEYS + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_KEYS) {
        printf("Resposta do servidor não foi a esperada\n %d", response->opcode);
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Alocar memória para a cópia das chaves (Verificar se não é preciso "+1")
    char **keys = malloc(((response->n_keys)+1) * sizeof(char *));
    if (keys == NULL) {
        printf("Erro ao alocar memória para as chaves\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Copiar as chaves da resposta
    for (size_t i = 0; i < response->n_keys; i++) {
        keys[i] = strdup(response->keys[i]);
        if (keys[i] == NULL) {
            printf("Erro ao duplicar a key\n");

            rtable_free_keys(keys);
            message_t__free_unpacked(response, NULL);
            return NULL;
        }

    }
    keys[response->n_keys] = NULL;  // Colocar o último elemento a NULL


    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);

    return keys;
}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys){

    if(keys ==NULL)
        return;

    for (int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
    }
    
    free(keys);
}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable){

    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_GETTABLE;
    msg.c_type=MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem\n");
        return NULL;
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_GETTABLE + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_TABLE) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Alocar memória para a cópia das chaves ((Verificar se não é preciso "+1"))
    struct entry_t **entries = malloc((response->n_entries+1) * sizeof(struct entry_t *));
    if (entries == NULL) {
        printf("Erro ao alocar memória para as entries\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Copiar as entries da resposta
    for (size_t i = 0; i < response->n_entries; i++) {
        

        struct block_t *result_block = rtable_get(rtable,response->entries[i]->key);
        if (result_block == NULL) {
            perror("Erro ao fazer get do block_t");
            message_t__free_unpacked(response, NULL);
            return NULL;
        }

        entries[i] = entry_create(strdup(response->entries[i]->key),result_block);
        if (entries[i] == NULL) {
            printf("Erro ao duplicar a entry\n");

            rtable_free_entries(entries);
            message_t__free_unpacked(response, NULL);
            return NULL;
        }
    }

    entries[response->n_entries] = NULL;  // Colocar o último elemento a NULL


    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);

    return entries;
    
}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries){

    if(entries ==NULL)
        return;

    for (int i = 0; entries[i] != NULL; i++) {
        entry_destroy(entries[i]);
    }
    
    free(entries);
}

/* Obtém as estatísticas do servidor. */
struct statistics_t *rtable_stats(struct rtable_t *rtable){

    MessageT msg;
    message_t__init(&msg);
    msg.opcode=MESSAGE_T__OPCODE__OP_STATS;
    msg.c_type=MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rtable, &msg);
    if (response == NULL) {
        printf("Erro ao enviar/receber mensagem\n");
        return NULL;
    }

    // Verificar se a mensagem de resposta é uma mensagem de erro
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Mensagem de ERRO recebida\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Verificar se veio a resposta esperada
    if (response->opcode != MESSAGE_T__OPCODE__OP_STATS + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_STATS) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Alocar e preencher `statistics_t`
    struct statistics_t *stats = malloc(sizeof(struct statistics_t));
    if (stats == NULL) {
        printf("Erro de memória.\n");
        return NULL;
    }
    stats->total_operations = response->stats->total_ops;
    stats->total_time = response->stats->total_time;
    stats->connected_clients = response->stats->active_clients;

    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);
    return stats;
}

/*
* Função que deveolve um par de rtables (head e tail) a quais cada cliente se liga.
* Aqui é feita a conexão do cliente com o Zookeeper, tal como a criação das duas rtables.
* Retorna o par de rtables ou NULL em caso de erro.
*/
struct rtable_pair_t *rtable_init(const char *zookeeper_address) {
    struct rtable_pair_t *rtable_pair = malloc(sizeof(struct rtable_pair_t));
    if (rtable_pair == NULL) {
        printf("Erro ao alocar memória para rtable_pair.\n");
        return NULL;
    }

    // Inicializar o contexto do watcher
    watcher_context_t *context = malloc(sizeof(watcher_context_t));
    if (context == NULL) {
        printf("Erro ao alocar memória para watcher_context_t.\n");
        free(rtable_pair);
        return NULL;
    }
    context->head =malloc(sizeof(struct rtable_t));
    context->tail = malloc(sizeof(struct rtable_t));

    // Conectar ao ZooKeeper
    if (zookeeper_connect(zookeeper_address, context) != 0) {
        printf("Erro ao conectar ao ZooKeeper.\n");
        free(rtable_pair);
        free(context);
        return NULL;
    }

    // Obter endereços do head e tail a partir do ZooKeeper
    char *head_address = NULL;
    char *tail_address = NULL;
    if (get_head_and_tail_addresses(&head_address, &tail_address) != 0) {
        printf("Erro ao obter endereços head e tail do ZooKeeper.\n");
        free(rtable_pair);
        free(context);
        free(head_address);
        free(tail_address);
        return NULL;
    }


    // Atualizar com o contexto com a conexão do head
    rtable_pair->head= context->head;
    printf("Conectado ao servidor head (%s).\n", head_address);

    // Atualizar o contexto com a conexão do tail
    rtable_pair->tail = context->tail;
    printf("Conectado ao servidor tail (%s).\n", tail_address);

    free(head_address);
    free(tail_address);

    return rtable_pair;
}