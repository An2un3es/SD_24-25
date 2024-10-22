
#include "block.h"
#include "entry.h"
#include "client_stub-private.h"
#include "client_network.h"


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
int rtable_put(struct rtable_t *rtable, struct entry_t *entry); // N esqeucer de chamar o client_network.send_receive

/* Retorna a entrada da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct block_t *rtable_get(struct rtable_t *rtable, char *key); // N esqeucer de chamar o client_network.send_receive

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key); // N esqeucer de chamar o client_network.send_receive

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable); // N esqeucer de chamar o client_network.send_receive

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable); // N esqeucer de chamar o client_network.send_receive

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys); // N esqeucer de chamar o client_network.send_receive

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable); // N esqeucer de chamar o client_network.send_receive

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries); // N esqeucer de chamar o client_network.send_receive
