
#include "block.h"
#include "entry.h"
#include "client_stub-private.h"
#include "client_network.h"
#include "htmessages.pb-c.h"


/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port){


    struct rtable_t *rtable = (struct rtable *)malloc(sizeof(struct rtable_t));

    if (rtable == NULL)
        return NULL;

    char *copy = strdup(address_port);
    char *host = strtok(copy, ":");
    copy = strtok(NULL, ":");
    char *port = strtok(copy, ":");

    rtable->server_address=host;
    rtable->server_port=atoi(&port);


    if (network_connect(rtable) < 0)
    {
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
    {
        return -1; // Retorna -1 em caso de erro
    } // TODO O que fazer no server? apagar tudo dps de desconectado?
    // bota fechar o socket

    if (network_close(rtable) == -1)
    {
        printf("error on disconecting");
        return -1;
    }
    // Resetar o descritor do socket na rtable
    rtable->sockfd = -1; //não sei quanto a necessidade disso
    free(rtable->server_address);
    free(rtable->server_port);
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

    return 0;


}

/* Retorna a entrada da tabela com chave key, ou NULL caso não exista
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
        return -1;  
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
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Alocar memória para a cópia das chaves
    char **keys = malloc((response->n_keys) * sizeof(char *));
    if (keys == NULL) {
        printf("Erro ao alocar memória para as chaves\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Copiar as chaves da resposta
    for (size_t i = 0; i < response->n_keys; i++) {
        keys[i] = strdup(response->keys[i]);
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
    if (response->opcode != MESSAGE_T__OPCODE__OP_GETTABLE + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_TABLE) {
        printf("Resposta do servidor não foi a esperada\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Alocar memória para a cópia das chaves
    struct entry_t **entries = malloc((response->n_entries) * sizeof(struct entry_t *));
    if (entries == NULL) {
        printf("Erro ao alocar memória para as entries\n");
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    // Copiar as entries da resposta
    for (size_t i = 0; i < response->n_entries; i++) {
        entries[i] = entry_duplicate(response->entries[i]);
        if (entries[i] == NULL) {
            printf("Erro ao duplicar a entry\n");

            for (size_t j = 0; j < i; j++) {
                entry_destroy(entries[j]);  
            }
            free(entries);
            message_t__free_unpacked(response, NULL);
            return NULL;
    }
    
    entries[response->n_entries] = NULL;  // Colocar o último elemento a NULL


    // Libertar a memória da resposta
    message_t__free_unpacked(response, NULL);

    return entries;

}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries); // N esqeucer de chamar o client_network.send_receive
