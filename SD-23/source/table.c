/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"


/*
*Função hash que agora já suporta UTF-8 
*/
int hash_code(char *key, int n) {
    setlocale(LC_ALL, ""); // Configura o programa para o ambiente de caracteres locais
    int total = 0;

    wchar_t wc;
    int bytes;
    while (*key != '\0') {

        bytes = mbtowc(&wc, key, MB_CUR_MAX);
        if (bytes == -1) {
            
            return -1;
        }
        total += wc;
        total = total % n;
        
        key += bytes; 
    }

    return total;
}


/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n){

    struct table_t *table = malloc(sizeof(struct table_t));

    if (table==NULL) {
        return NULL; 
    }

    table->size = n;
    table->lists = malloc(n * sizeof(struct list_t *));

    if (table->lists==NULL) {
        free(table); 
        return NULL; 
    }

    
    for (int i = 0; i < n; i++) {
        table->lists[i] = list_create();
    }

    return table; 
}

/* Função para adicionar um par chave-valor à tabela. Os dados de entrada
 * desta função deverão ser copiados, ou seja, a função vai criar uma nova
 * entry com *CÓPIAS* da key (string) e dos dados. Se a key já existir na
 * tabela, a função tem de substituir a entry existente na tabela pela
 * nova, fazendo a necessária gestão da memória.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *t, char *key, struct block_t *value){

    if (t == NULL || value == NULL || key == NULL)
        return -1;

    int index = hash_code(key, t->size);
    struct list_t *list = t->lists[index];

    if (list == NULL)
        return -1;

    // Cria uma cópia da chave
    char *key_copy = strdup(key);
    if (key_copy == NULL) {
        return -1; 
    }

    // Cria uma cópia do bloco usando block_duplicate
    struct block_t *value_copy = block_duplicate(value);
    if (value_copy == NULL) {
        free(key_copy); 
        return -1; 
    }

    // Cria a nova entry
    struct entry_t *entry = entry_create(key_copy, value_copy);
    if (entry == NULL) {
        free(key_copy); 
        block_destroy(value_copy); 
        return -1; 
    }

    // Adiciona a entry à lista
    if (list_add(list, entry) == -1) {  
        entry_destroy(entry); 
        return -1;
    }

    return 0;
}

/* Função que procura na tabela uma entry com a chave key. 
 * Retorna uma *CÓPIA* dos dados (estrutura block_t) nessa entry ou 
 * NULL se não encontrar a entry ou em caso de erro.
 */
struct block_t *table_get(struct table_t *t, char *key){

    if (t==NULL || key ==NULL)
        return NULL;
    
    int index = hash_code(key,t->size);
    struct list_t *list= t->lists[index];

    struct entry_t *entry_real =list_get(list,key);
    struct entry_t *entry_copia = entry_duplicate(entry_real);
    
    if(entry_copia==NULL)
        return NULL;
    
    return entry_copia->value;

}
/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *t){

    if(t==NULL)
        return -1;
    
    int i=0;
    int count=0;

    while(i<t->size){
        struct list_t * list = t->lists[i];
        if(list==NULL)
            return -1;
        
        count=count+list->size;
        i++;
    }

    return count;

}

/* Função auxiliar que constrói um array de char* com a cópia de todas as keys na 
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **table_get_keys(struct table_t *t){
    if (t == NULL) {
        return NULL;
    }


    int total_keys = table_size(t);
    if (total_keys == -1) {
        return NULL;
    }

    char **keys_array_table = (char **)malloc((total_keys + 1) * sizeof(char *));
    if (keys_array_table == NULL) {
        return NULL; 
    }

    int j = 0; 


    for (int i = 0; i < t->size; i++) {
        struct list_t *list = t->lists[i];

        char **keys_array_lista = list_get_keys(list);
        if (keys_array_lista != NULL) {
            
            for (int k = 0; keys_array_lista[k] != NULL; k++) {
                keys_array_table[j++] = keys_array_lista[k];
            }
            free(keys_array_lista); 
        }
    }

    keys_array_table[j] = NULL;

    return keys_array_table;

    
}

/* Função auxiliar que liberta a memória ocupada pelo array de keys obtido pela 
 * função table_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_free_keys(char **keys){

    if(keys ==NULL)
        return -1;

    for (int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
    }
    
    free(keys);

    return 0;

}

/* Função que remove da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int table_remove(struct table_t *t, char *key){

    if(t==NULL || key ==NULL)
        return -1;

    int value =hash_code(key,t->size);

    struct list_t *lista= t->lists[value];
    
    return list_remove(lista,key);

}

/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *t){

    if(t==NULL || t->lists<0|| t->lists==NULL)
        return -1;
    
    for(int i =0; i<t->size; i++){
        struct list_t * lista =t->lists[i];
        list_destroy(lista);
    }

    free(t);
    return 0;


}
 
