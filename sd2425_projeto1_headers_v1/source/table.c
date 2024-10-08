

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"



unsigned int hash (char *key, struct table_t *t){
    int size= strlen(key);
    unsigned int total = 0;
    for(int i =0; i<size;i++){
        total+= key[i];
        total= total % t->n_linhas;
    }

    return total;
}

/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n){

    struct list_t *l [n];
    if (n==0)
        return NULL;
    
    struct table_t *t = (struct table_t *) malloc(sizeof(struct table_t));

    t->listas=l;
    t->n_linhas=n;

    for (int i=0; i<n;i++){
        t->listas[i]=NULL;
    }

    return t;
}

/* Função para adicionar um par chave-valor à tabela. Os dados de entrada
 * desta função deverão ser copiados, ou seja, a função vai criar uma nova
 * entry com *CÓPIAS* da key (string) e dos dados. Se a key já existir na
 * tabela, a função tem de substituir a entry existente na tabela pela
 * nova, fazendo a necessária gestão da memória.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *t, char *key, struct block_t *value){
    if (t==NULL || value==NULL || key ==NULL)
        return -1;
    
    int index = hash(key,t);
    struct list_t *list= t->listas[index];

    if(list==NULL)
        return -1;

    struct entry_t *entry =entry_create(key,value);
    if(entry==NULL){
        entry_destroy(entry);
        return -1;
    }
    list_add(list,entry);

    return 0;
}

/* Função que procura na tabela uma entry com a chave key. 
 * Retorna uma *CÓPIA* dos dados (estrutura block_t) nessa entry ou 
 * NULL se não encontrar a entry ou em caso de erro.
 */
struct block_t *table_get(struct table_t *t, char *key){

    if (t==NULL || key ==NULL)
        return NULL;
    
    int index = hash(key,t);
    struct list_t *list= t->listas[index];

    struct entry_t *entry =list_get(list,key);
    if(entry==NULL)
        return NULL;
    
    return entry->value;

}
/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *t){

    if(t==NULL)
        return -1;
    
    int i=0;
    int count;

    while(i<t->n_linhas){
        struct list_t * list = t->listas[i];
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


    for (int i = 0; i < t->n_linhas; i++) {
        struct list_t *list = t->listas[i];

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

    int value =hash(key,t);

    struct list_t *lista= t->listas[value];
    
    return list_remove(lista,key);

}

/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *t){
    if(t==NULL)
        return -1;
    
    for(int i =0; i<t->n_linhas;i++){

        list_destroy(t->listas[i]);
    }

    free(t);

    return 0;


}
 
