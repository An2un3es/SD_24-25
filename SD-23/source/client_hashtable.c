/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <inttypes.h>
#include "client_stub.h"
#include "client_stub-private.h"
#include "message-private.h"
#include "stats.h"
#include <zookeeper/zookeeper.h>


int main(int argc, char **argv) {

    signal(SIGPIPE, SIG_IGN);  // Ignorar SIGPIPE

    if (argc < 2) {
        printf("Exemplo: %s <zookeeper_IP:zookeeper_porto>\n", argv[0]);
        return -1;
    }

    const char *zookeeper_address = argv[1];


    // Inicializar conexão ao ZooKeeper e servidores head/tail
    struct rtable_pair_t *rtable_pair = rtable_init(zookeeper_address);
    if (rtable_pair == NULL) {
        printf("Erro ao inicializar o par head/tail.\n");
        return -1;
    }


    printf("Comandos: \n put <key> <value> -> inserir o dado na tabela com a chave dada\n get <key> -> retornar o conteudo da chave pedida, se existir \n del <key> -> remover a entry com a chave dada da tabela\n size -> retornar o número de entrys na tabela \n getkeys -> retornar todas as chaves existentes \n gettable -> retornar todas as entrys na tabela\n stats -> obter estatisticas do servidor\n quit -> encerrar ligação com o servidor\n");
    char input[256];
    while (1) {
        printf("Insira um Comando: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Remover o '\n' da entrada
        input[strcspn(input, "\n")] = 0;

        // Separar o comando e os argumentos
        char *command = strtok(input, " ");
        if (command == NULL) continue;

        //----------------------------------------------

        if (strcmp(command, "put") == 0) {
            char *key = strtok(NULL, " ");
            char *value = strtok(NULL, "");
            
            if (key && value) {
                // Cria uma cópia do valor
                char *value_copy = strdup(value);
                if (!value_copy) {
                    printf("Erro ao alocar memória para o value.\n");
                    continue;  
                }

                // Cria o block
                struct block_t *value_block = block_create(strlen(value_copy) + 1, value_copy);
                if (!value_block) {
                    printf("Erro ao criar o bloco.\n");
                    free(value_copy);
                    continue;  
                }

                // Cria a entry
                struct entry_t *entry = entry_create(key, value_block);
                if (!entry) {
                    printf("Erro ao criar a entrada.\n");
                    block_destroy(value_block);
                    continue; 
                }

                int result = rtable_put(rtable_pair->head, entry);
               
                printf("put: %s\n", result == 0 ? "sucesso" : "falhou");
                free(value_copy);

            } else {
                printf("Exemplo: put <key> <value>\n");
            }

        //----------------------------------------------

        } else if (strcmp(command, "get") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                struct block_t *value = rtable_get(rtable_pair->tail, key);
                if(value==NULL){
                    printf("Dados não encontrados\n");
                }else{
                    printf( "Data: %p\nDatasize: %d\n", value->data, value->datasize);
                    block_destroy(value);
                }
            } else {
                printf("Exemplo: get <key>\n");
            }
        
        //----------------------------------------------

        } else if (strcmp(command, "del") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                int result = rtable_del(rtable_pair->head, key);
                printf("del: %s\n", result == 0 ? "entry deletada com sucesso" : "falhou ou não foi encontrado");
            } else {
                printf("Exemplo: del <key>\n");
            }

        //----------------------------------------------

        } else if (strcmp(command, "size") == 0) {

            char *badsintax = strtok(NULL, " ");
            if (badsintax){
                 printf("Uso: size \n");
                 continue;
            }
           int size = rtable_size(rtable_pair->tail);

            printf("size: %d\n", size);

        //----------------------------------------------

        } else if (strcmp(command, "getkeys") == 0) {
            char *badsintax = strtok(NULL, " ");
            if (badsintax){
                 printf("Uso: getkeys\n");
                 continue;
            }
            char ** keys=rtable_get_keys(rtable_pair->tail);

            if (keys == NULL) {
                printf("Erro ao conseguir as keys.\n");
                continue;
            }

            if (keys[0]==NULL){ // *k 
                printf("Tabela vazia.\n");
                continue;
            }

            printf("Keys da Tabela:\n");
            for (int i = 0; keys[i] != NULL; i++) {
                if (keys[i]!=NULL){
                    printf("-> %s\n", keys[i]);
                }else{
                    printf("-> NULL");
                }
            }
            rtable_free_keys(keys);

        //----------------------------------------------
        } else if (strcmp(command, "gettable") == 0) {
            char *badsintax = strtok(NULL, " ");
            if (badsintax){
                 printf("Uso: gettable\n");
                 continue;
            }
            struct entry_t **entrys=rtable_get_table(rtable_pair->tail);
            if (entrys == NULL) {
                printf("Erro ao conseguir as entrys.\n");
                continue;
            }
            if(entrys[0] == NULL){
                printf("Tabela vazia.\n");
                continue;
            }
            printf("Entries da Tabela:\n");
            for (int i = 0; entrys[i] != NULL; i++) {
                printf("Entry nº %d: Key->%s | Data-> %p - Datasize-> %d\n", i,entrys[i]->key, entrys[i]->value->data,entrys[i]->value->datasize);
                
            }
            rtable_free_entries(entrys);


        }else if(strcmp(command, "stats") == 0){
            char *badsintax = strtok(NULL, " ");
            if (badsintax){
                 printf("Uso: stats\n");
                 continue;
            }

            struct statistics_t* stats=rtable_stats(rtable_pair->tail);
            if (stats == NULL) {
                printf("Erro ao conseguir estatisticas.\n");
                continue;
            }

            printf("Estatisticas na tabela:\n Número total de operações executadas-> %u\n Tempo gasto em operações-> %lu microssegundos\n Número de clientes ligados-> %u\n", stats->total_operations,stats->total_time,stats->connected_clients);
            

            free(stats);

            
        } else if (strcmp(command, "quit") == 0) {
            break;
        } else {
            printf("Comando desconhecido: %s\n", command);
        }
    }
    // Fechar a conexão com o servidor
    rtable_disconnect(rtable_pair->head);
    rtable_disconnect(rtable_pair->tail);
    free (rtable_pair);
    return 0;
}
