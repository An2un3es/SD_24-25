/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"
#include "htmessages.pb-c.h"
#include "block.h"

EntryT *convert_to_entry_t(struct entry_t *entry);

struct entry_t **list_get_entries(struct list_t *list);

/* Inicia o skeleton da tabela. 
* O main() do servidor deve chamar esta função antes de poder usar a * função invoke(). O parâmetro n_lists define o número de listas a
* serem usadas pela tabela mantida no servidor.
* Retorna a tabela criada ou NULL em caso de erro. */ 
struct table_t *server_skeleton_init(int n_lists){

    if(n_lists<=0)
        return NULL;
    struct table_t *table =table_create(n_lists);
    if(table==NULL)
        return NULL;
    
    return table;
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
* e outros recursos usados pelo skeleton.
* Retorna 0 (OK) ou -1 em caso de erro. */
int server_skeleton_destroy(struct table_t *table){
    if(table == NULL)
        return -1;

    table_destroy(table);

    return 0;
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
* e utiliza a mesma estrutura MessageT para devolver o resultado. * Retorna 0 (OK) ou -1 em caso de erro. */ 
int invoke(MessageT *msg, struct table_t *table){

    if (msg==NULL || table==NULL){
        return -1;
    }
    
    switch (msg->opcode)
    {
        case MESSAGE_T__OPCODE__OP_PUT:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_ENTRY) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            
            // Criar um novo block com key e value
            struct block_t *new_block = block_create(msg->entry->value.len, "");
            new_block->data = malloc(msg->entry->value.len);
            memcpy(new_block->data, msg->entry->value.data, msg->entry->value.len);
        
            if (new_block == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            //Adiciona a entry na tabela
            int result1 = table_put(table, msg->entry->key ,new_block);

            // Define a resposta com base no resultado
            if (result1 == 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Limpa a memória temporária
            block_destroy(new_block);
            printf("Comando put executado com sucesso\n");
            //NUMERO DE  OPERAÇÕES AUMENTA
            break;
        
        case MESSAGE_T__OPCODE__OP_GET:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação Get na tabela
            struct block_t *block = table_get(table, msg->key);

            if (block == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }else {

                msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->value.len = block->datasize;
                msg->value.data = block->data;
            }
            printf("Comando get executado com sucesso\n");
            //NUMERO DE  OPERAÇÕES AUMENTA
            break;

        case MESSAGE_T__OPCODE__OP_DEL:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação Del na tabela
            int result2 = table_remove(table, msg->key);

            if (result2 == 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            printf("Comando del executado com sucesso\n");
            //NUMERO DE  OPERAÇÕES AUMENTA
            break;
        case MESSAGE_T__OPCODE__OP_SIZE:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação Size na tabela
            int result3 = table_size(table);

            if (result3 < 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->result=result3;
            }
            printf("Comando size executado com sucesso\n");
            //NUMERO DE  OPERAÇÕES AUMENTA
            break;

        case MESSAGE_T__OPCODE__OP_GETKEYS:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação GetKeys na tabela
            char **keys = table_get_keys(table);

            if (keys ==NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
                msg->keys=keys;
                msg-> n_keys = table_size(table);
            }
            printf("Comando getkeys executado com sucesso\n");
            //NUMERO DE  OPERAÇÕES AUMENTA
            break;
        case MESSAGE_T__OPCODE__OP_GETTABLE:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Contar o total de entradas
            int total_entries = table_size(table);
           

            EntryT **all_entries = malloc((total_entries + 1) * sizeof(EntryT *));
            if (all_entries == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            msg->n_entries = 0;
            for (int i = 0; i < table->size; i++) {
                struct entry_t **list_entries = list_get_entries(table->lists[i]);
                if (list_entries != NULL) {
                    for (int j = 0; list_entries[j] != NULL; j++) {
                        EntryT *converted_entry = convert_to_entry_t(list_entries[j]);
                        if (converted_entry != NULL) {
                            all_entries[msg->n_entries++] = converted_entry;
                        } else {
                            
                            for (int k = 0; k < msg->n_entries; k++) {
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
                    free(list_entries);
                }
            }

            all_entries[msg->n_entries] = NULL;  // Terminar com NULL

            msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
            msg->entries = all_entries;
            printf("Comando gettable executado com sucesso. Quantidade de operações gets que serão executadas: %d\n", table_size(table));
            //NUMERO DE  OPERAÇÕES AUMENTA
            break;

        case MESSAGE_T__OPCODE__OP_STATS:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }


            break;
        
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


    return 0; 


}


/*
* "Converte" entry_y para EntryT
*/
EntryT *convert_to_entry_t(struct entry_t *entry) {
    if (entry == NULL) return NULL;

    // Alocar EntryT 
    EntryT *entry_t = malloc(sizeof(EntryT));
    if (entry_t == NULL) {
        return NULL;
    }

    // Inicializar EntryT
    entry_t__init(entry_t);

    // Copiar a chave
    entry_t->key = strdup(entry->key);
    if (entry_t->key == NULL) {
        free(entry_t);
        return NULL;
    }

    // Configurar value com len e alocar espaço para data
    entry_t->value.len = entry->value->datasize;
    entry_t->value.data = malloc(entry->value->datasize);
    if (entry_t->value.data == NULL) {
        free(entry_t->key);
        free(entry_t);
        return NULL;
    }

    // Copiar os dados
    memcpy(entry_t->value.data, entry->value->data, entry->value->datasize);

    return entry_t;
}
