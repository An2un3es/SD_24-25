/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"
#include "htmessages.pb-c.h"
#include "block.h"
#include "stats-private.h"
#include "zookeeper_usage.h"

statistics_t server_stats;

EntryT *convert_to_entry_t(struct entry_t *entry);

struct entry_t **list_get_entries(struct list_t *list);

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro. */
struct table_t *server_skeleton_init(int n_lists)
{

    if (n_lists <= 0)
        return NULL;
    struct table_t *table = table_create(n_lists);
    if (table == NULL)
        return NULL;

    // Inicializar os campos de server_stats
    server_stats.total_operations = 0;
    server_stats.total_time = 0;
    server_stats.connected_clients = 0;
    pthread_mutex_init(&server_stats.stats_mutex, NULL); // Inicializar o mutex

    return table;
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro. */
int server_skeleton_destroy(struct table_t *table)
{
    if (table == NULL)
        return -1;

    table_destroy(table);

    // Destruir o mutex de server_stats
    pthread_mutex_destroy(&server_stats.stats_mutex);

    return 0;
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
 * e utiliza a mesma estrutura MessageT para devolver o resultado. * Retorna 0 (OK) ou -1 em caso de erro. */
int invoke(MessageT *msg, struct table_t *table)
{

    if (msg == NULL || table == NULL)
    {
        return -1;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_PUT:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_ENTRY)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Criar um novo block com key e value
        struct block_t *new_block = block_create(msg->entry->value.len, "");
        new_block->data = malloc(msg->entry->value.len);
        memcpy(new_block->data, msg->entry->value.data, msg->entry->value.len);

        if (new_block == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        // Adiciona a entry na tabela
        int result1 = table_put(table, msg->entry->key, new_block);
        gettimeofday(&end, NULL);

        // Define a resposta com base no resultado
        if (result1 == 0)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }


        if(rtable_put_next(msg->entry->key,new_block)<0){
            printf("ERROOOOOOOOOOOO\n");
        }

        // Limpa a memória temporária
        block_destroy(new_block);
        printf("Comando put executado com sucesso\n");

        break;

    case MESSAGE_T__OPCODE__OP_GET:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Executa a operação Get na tabela
        struct block_t *block = table_get(table, msg->key);
        gettimeofday(&end, NULL);

        if (block == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        else
        {

            msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->value.len = block->datasize;
            msg->value.data = block->data;
        }
        free(block);
        printf("Comando get executado com sucesso\n");
        break;

    case MESSAGE_T__OPCODE__OP_DEL:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Executa a operação Del na tabela
        int result2 = table_remove(table, msg->key);
        gettimeofday(&end, NULL);

        if (result2 == 0)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        if(rtable_del_next(msg->key)<0){
            printf("ERROOOOOOOOOOOO\n");
        }
        printf("Comando del executado com sucesso\n");

        break;
    case MESSAGE_T__OPCODE__OP_SIZE:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Executa a operação Size na tabela
        int result3 = table_size(table);
        gettimeofday(&end, NULL);

        if (result3 < 0)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = result3;
        }
        printf("Comando size executado com sucesso\n");

        break;

    case MESSAGE_T__OPCODE__OP_GETKEYS:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Executa a operação GetKeys na tabela
        char **keys = table_get_keys(table);
        gettimeofday(&end, NULL);

        if (keys == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->keys = keys;
            msg->n_keys = table_size(table);
        }
        printf("Comando getkeys executado com sucesso\n");

        break;
    case MESSAGE_T__OPCODE__OP_GETTABLE:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Contar o total de entradas
        int total_entries = table_size(table);

        EntryT **all_entries = malloc((total_entries + 1) * sizeof(EntryT *));
        if (all_entries == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        msg->n_entries = 0;
        for (int i = 0; i < table->size; i++)
        {
            struct entry_t **list_entries = list_get_entries(table->lists[i]);
            if (list_entries != NULL)
            {
                for (int j = 0; list_entries[j] != NULL; j++)
                {
                    EntryT *converted_entry = convert_to_entry_t(list_entries[j]);
                    if (converted_entry != NULL)
                    {
                        all_entries[msg->n_entries++] = converted_entry;
                    }
                    else
                    {

                        for (int k = 0; k < msg->n_entries; k++)
                        {
                            free(all_entries[k]->key);
                            free(all_entries[k]->value.data);
                            free(all_entries[k]);
                        }
                        free(all_entries);
                        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                        return -1;
                    }
                }
                for (int j = 0; list_entries[j] != NULL; j++)
                    entry_destroy(list_entries[j]);
                free(list_entries);
            }
        }

        all_entries[msg->n_entries] = NULL; // Terminar com NULL

        msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
        msg->entries = all_entries;
        printf("Comando gettable executado com sucesso. Quantidade de operações gets que serão executadas: %d\n", table_size(table));

        gettimeofday(&end, NULL);
        pthread_mutex_lock(&server_stats.stats_mutex);
        server_stats.total_operations -= total_entries; // diminuir operações em gettable por causa dos gets
        pthread_mutex_unlock(&server_stats.stats_mutex);
        break;

    case MESSAGE_T__OPCODE__OP_STATS:

        if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }

        // Alocar e inicializar msg->stats
        msg->stats = malloc(sizeof(StatsT));
        if (msg->stats == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        stats_t__init(msg->stats);

        // Bloquear o mutex e preencher os dados de stats
        pthread_mutex_lock(&server_stats.stats_mutex);
        msg->stats->total_ops = server_stats.total_operations;
        msg->stats->total_time = server_stats.total_time;
        msg->stats->active_clients = server_stats.connected_clients;
        pthread_mutex_unlock(&server_stats.stats_mutex);

        msg->opcode = MESSAGE_T__OPCODE__OP_STATS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;

        printf("Comando stats executado com sucesso\n");
        return 0;

    case MESSAGE_T__OPCODE__OP_BAD:

        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

        return -1;

    case MESSAGE_T__OPCODE__OP_ERROR:

        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;

    default:

        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

        return -1;
    }

    // Calcular o tempo da operação e atualizar as estatísticas
    uint64_t op_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.total_operations++;    // Incrementar operações
    server_stats.total_time += op_time; //  Tempo
    pthread_mutex_unlock(&server_stats.stats_mutex);

    return 0;
}

/*
 * "Converte" entry_y para EntryT
 */
EntryT *convert_to_entry_t(struct entry_t *entry)
{
    if (entry == NULL)
        return NULL;

    // Alocar EntryT
    EntryT *entry_t = malloc(sizeof(EntryT));
    if (entry_t == NULL)
    {
        return NULL;
    }

    // Inicializar EntryT
    entry_t__init(entry_t);

    // Copiar a chave
    entry_t->key = strdup(entry->key);
    if (entry_t->key == NULL)
    {
        free(entry_t);
        return NULL;
    }

    // Configurar value com len e alocar espaço para data
    entry_t->value.len = entry->value->datasize;
    entry_t->value.data = malloc(entry->value->datasize);
    if (entry_t->value.data == NULL)
    {
        free(entry_t->key);
        free(entry_t);
        return NULL;
    }

    // Copiar os dados
    memcpy(entry_t->value.data, entry->value->data, entry->value->datasize);

    return entry_t;
}
