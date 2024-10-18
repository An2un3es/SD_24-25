#include <stdio.h>
#include <string.h>
#include "client_stub.h"
#include "client_network.h" // Inclui as funções de rede
#include "message-private.h"

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Uso: %s <server> <port>\n", argv[0]);
        return -1;
    }

    // Inicializar a tabela remota (estrutura rtable_t)
    struct rtable_t *rtable = rtable_bind(argv[1], atoi(argv[2]));
    if (rtable == NULL) {
        fprintf(stderr, "Erro ao conectar ao servidor.\n");
        return -1;
    }

    char input[256];
    while (1) {
        printf("Comando: ");
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

                int result = rtable_put(rtable, entry);
                printf("put: %s\n", result == 0 ? "sucesso" : "falhou");
                entry_destroy(entry);
                free(value_copy);

            } else {
                printf("Uso: put <key> <value>\n");
            }

        //----------------------------------------------

        } else if (strcmp(command, "get") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                char *value = rtable_get(rtable, key);
                printf("get: %s\n", value ? value : "não encontrado");
            } else {
                printf("Uso: get <key>\n");
            }
        
        //----------------------------------------------

        } else if (strcmp(command, "del") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                int result = rtable_del(rtable, key);
                printf("del: %s\n", result == 0 ? "sucesso" : "falhou");
            } else {
                printf("Uso: del <key>\n");
            }

        //----------------------------------------------

        } else if (strcmp(command, "size") == 0) {
            int size = rtable_size(rtable);
            printf("size: %d\n", size);

        //----------------------------------------------

        } else if (strcmp(command, "getkeys") == 0) {

            char ** keys=rtable_get_keys(rtable);

            if (keys == NULL) {
            printf("Array está vazio.\n");
            }
            printf("Keys da Tabela:\n");
            for (int i = 0; keys[i] != NULL; i++) {
                printf("-> %s\n", keys[i]);
            }
        } else if (strcmp(command, "gettable") == 0) {

            struct entry_t **entrys=rtable_get_table(rtable);
            if (entrys == NULL) {
            printf("Array está vazio.\n");
            }
            printf("Entries da Tabela:\n");
            for (int i = 0; entrys[i] != NULL; i++) {
                printf("Entry nº %d: Key->%s | Datasize-> %d\n", i,entrys[i]->key, entrys[i]->value->datasize);
            }
            
        } else if (strcmp(command, "quit") == 0) {
            break;
        } else {
            printf("Comando desconhecido: %s\n", command);
        }
    }

    // Fechar a conexão com o servidor
    rtable_unbind(rtable);
    return 0;
}
