
#include "table.h"
#include "htmessages.pb-c.h"

/* Inicia o skeleton da tabela. 
* O main() do servidor deve chamar esta função antes de poder usar a * função invoke(). O parâmetro n_lists define o número de listas a
* serem usadas pela tabela mantida no servidor.
* Retorna a tabela criada ou NULL em caso de erro. */ 
struct table_t *server_skeleton_init(int n_lists){

    if(n_lists==NULL || n_lists<=0)
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
            
            // Criar uma nova entrada com key e value
            struct block_t *block = block_create(msg->entry->value.len, msg->entry->value.data);
            
            if (block == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação PUT na tabela
            int result = table_put(table, msg->entry->key ,block);

            // Define a resposta com base no resultado
            if (result == 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }

            // Limpa a memória temporária
            block_destroy(block);

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
            }else {

                msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->value.len = block->datasize;
                msg->value.data = block->data;
            }

            break;

        case MESSAGE_T__OPCODE__OP_DEL:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação Get na tabela
            int result = table_remove(table, msg->key);

            if (result == 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }


            break;
        case MESSAGE_T__OPCODE__OP_SIZE:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação Get na tabela
            int result = table_size(table);

            if (result < 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->result=result;
            }
            break;

        case MESSAGE_T__OPCODE__OP_GETKEYS:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Executa a operação Get na tabela
            char **keys = table_get_keys(table);

            if (keys ==NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;  
                msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
                msg->keys=keys;
            }

            break;
        case MESSAGE_T__OPCODE__OP_GETTABLE:

            if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            // Contar o total de entradas
            int total_entries = 0;
            int i = 0;
            while (table->lists[i] != NULL) {
                total_entries += table->lists[i]->size;
                i++;
            }

            struct entry_t **all_entries = malloc((total_entries + 1) * sizeof(struct entry_t *));
            if (all_entries == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            int index = 0;
            for (i = 0; i < table->size; i++) {
                struct entry_t **list_entries = list_get_entries(table->lists[i]);
                if (list_entries == NULL) {
                    for (int j = 0; j < index; j++) {
                        entry_destroy(all_entries[j]);
                    }
                    free(all_entries);
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    return -1;
                }

                // Copiar as entradas obtidas da lista para o array all_entries
                for (int j = 0; list_entries[j] != NULL; j++) {
                    all_entries[index++] = list_entries[j];
                }
                free(list_entries);  // Libertar o array temporário (mas não as entradas)
            }
            all_entries[index] = NULL;  // Colocar o último elemento a NULL

            // Configurar a mensagem de resposta com o array de entries
            msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
            msg->n_entries = index;
            msg->entries = all_entries;

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