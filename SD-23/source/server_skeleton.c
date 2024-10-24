
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

    if (msg==NULL || table==NULL)
        return -1;
    
    switch (msg->opcode)
    {
        case MESSAGE_T__OPCODE__OP_PUT:

            if(msg->c_type!=MESSAGE_T__C_TYPE__CT_ENTRY)
                return -1;
            
            int i = entry_t__get_packed_size(msg->entry);
            struct block_t *block=block_create(i,msg->entry->value);

            if(table_put(table,msg->entry->key,block)<-1){
                return -1;
            }
                

        

        case MESSAGE_T__OPCODE__OP_GET:







    }


    return 0;


}


