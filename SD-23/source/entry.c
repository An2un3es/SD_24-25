/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
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

    if (key == NULL || value == NULL || value->data == NULL || value ->datasize <= 0) {
        return NULL;  
    }
    struct entry_t *entry=(struct entry_t *) malloc(sizeof(struct entry_t));
    if(entry == NULL)
        return NULL;
    entry->key=key;
    entry->value=value;
    return entry;

}
/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se e1 < e2,
 * 1 se e1 > e2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *e1, struct entry_t *e2){
    
    if(e1==NULL || e1->key == NULL || e1->value == NULL || e2==NULL || e2->key == NULL || e2 ->value == NULL)
        return -2;
   
    int output = strcmp (e1->key,e2->key);

    if(output==0){
        return 0;
    }else if(output>0){
        return 1;
    }else
        return -1;
    

    return -2;
    
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

    if(e==NULL || e->key == NULL || e->value == NULL || new_key==NULL || new_value==NULL){
        return -1;
    }
    if(e->key != new_key){
        free(e->key);
        e->key=new_key;
    }
    if(e->value != new_value){
        free(e->value);
        e->value=new_value;
    }
    return 0;
    

}

/* Função que elimina uma entry, libertando a memória por ela ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_destroy(struct entry_t *e){
    if(e==NULL || e->key == NULL || e->value == NULL)
        return -1;
    free(e->key);
    block_destroy(e->value);
    free(e);
    return 0;
}

