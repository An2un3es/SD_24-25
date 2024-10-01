
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entry.h"
#include "block.h"



/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados de entrada.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_create(char *key, struct block_t *value){

    if (key == NULL || value == NULL) {
        return NULL;  
    }

    struct entry_t *entry=(struct entry_t *) malloc(sizeof(struct entry_t));
    if(entry ==NULL)
        return NULL;
    
    entry->key=strdup(key);
    if(entry->key==NULL){
        free(entry);
        return NULL;
    }
    entry->value=block_duplicate(value);
    if(entry->value==NULL){
        free(entry);
        return NULL;
    }


    return entry;

}

/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se e1 < e2,
 * 1 se e1 > e2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *e1, struct entry_t *e2){
    
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_duplicate(struct entry_t *e){
     
    if(e==NULL)
        return NULL;
    
    struct entry_t *new_entry = (struct entry_t *)malloc(sizeof(struct entry_t));

     if(new_entry==NULL)
        return NULL;
    
    new_entry->key=strdup(e->key);
    if(new_entry->key==NULL){
        free(new_entry);
        return NULL;
    }
    

    new_entry->value=block_duplicate(e->value);
    if(new_entry->value==NULL){
        free(new_entry);
        return NULL;
    }

    return new_entry;
}

/* Função que substitui o conteúdo de uma entry, usando a nova chave e
 * o novo valor passados como argumentos, e eliminando a memória ocupada
 * pelos conteúdos antigos da mesma.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_replace(struct entry_t *e, char *new_key, struct block_t *new_value){

    if(e==NULL || new_key==NULL || new_value==NULL){
        return -1;
    }

    if(e->key!=NULL)
        free(e->key);

    e->key=strdup(new_key);

    if(e->value!=NULL)
        free(e->value);
    
    e->value=block_duplicate(new_value);

    return 0;

}

/* Função que elimina uma entry, libertando a memória por ela ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_destroy(struct entry_t *e){
    
    if(e==NULL)
        return -1;
    
    if(e->key!=NULL)
        free(e->key);
    if(e->value!=NULL)
        block_destroy(e->value);

    free(e);
    return 0;
}

