/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "serialization.h"

const int ERROR_SERIALIZATION = -1;


/* Serializa todas as chaves presentes no array de strings keys para o
* buffer keys_buf, que será alocado dentro da função. A serialização
* deve ser feita de acordo com o seguinte formato:
* | int | string | string | string |
* | nkeys | key1 | key2 | key3 |
* Retorna o tamanho do buffer alocado ou -1 em caso de erro.
*/
int keyArray_to_buffer(char **keys, char **keys_buf){ 
    
      if (keys == NULL || keys_buf == NULL) { 
        return ERROR_SERIALIZATION;
    }
    // Para seguir o formato pedido:
    // 1 Contar o número de chaves
    int nkeys = 0;
    while (keys[nkeys] != NULL) {
        nkeys++;
    }
    if (nkeys == 0)
        return -1; //array vazio... 
    // 2  Calcular o tamanho necessário para o buffer

    int buffer_size = sizeof(int); // nkeys
    for (int i = 0; i < nkeys; i++) {
        buffer_size += strlen(keys[i]) + 1; // +1 para incluir o terminador nulo de cada string
    }

    // Alocar o buffer
    *keys_buf = (char *)malloc(buffer_size);
    if (*keys_buf == NULL) {
        return ERROR_SERIALIZATION; // Falha na alocação!
    }
    // Preencher o buffer
    char *ptr = *keys_buf;
    int nkeys_network_order = htonl(nkeys);
    memcpy(ptr, &nkeys_network_order, sizeof(int)); // Serializar o número de chaves (nkeys) 
    ptr += sizeof(int); // move o ponteiro a frente!
    // Serializar cada string (key)
    for (int i = 0; i < nkeys; i++) {
        strcpy(ptr, keys[i]); // Copiar a string incluindo o terminador nulo
        ptr += strlen(keys[i]) + 1;
    }

    return buffer_size;
}
/* De-serializa a mensagem contida em keys_buf, colocando-a num array de
* strings cujo espaco em memória deve ser reservado. A mensagem contida
* em keys_buf deverá ter o seguinte formato:
* | int | string | string | string |
* | nkeys | key1 | key2 | key3 |
* Retorna o array de strings ou NULL em caso de erro.
*/
char** buffer_to_keyArray(char *keys_buf){

    if (keys_buf == NULL) 
        return NULL; // Buffer inválido
   
    int nkeys;
    memcpy(&nkeys,keys_buf,sizeof(int));
    nkeys = ntohl(nkeys);
    if(nkeys <= 0)
        return NULL; // numero invalido, temde haver ao menos 1 key!

    char ** keys_array = (char **) malloc((nkeys + 1) * sizeof(char *)); //alocar memoria para o array todo
    if (keys_array == NULL)
        return NULL; // falha alocação

    

    char * ptr = keys_buf + sizeof(int);

    //bota desserializar
    for(int i = 0; i <nkeys;i++){
        int len = strlen(ptr);
        keys_array[i] = (char *) malloc((len + 1)* sizeof(char));
        if(keys_array[i] == NULL){
            //Deu ruim pessoal, falha na alocação, temos que liberar tudinho
            for(int j = 0; j< i; j++)
                free(keys_array[j]);
            free (keys_array);
            return NULL;
        }
        strcpy(keys_array[i],ptr);
        ptr += len + 1; //mover o ptr para a prox string

    }
    
    keys_array[nkeys] = NULL;
    return keys_array;


}
