/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "entry.h"
#include "list.h"
#include "list-private.h"

struct list_t; /* a definir pelo grupo em list-private.h */

/* Função que cria e inicializa uma nova lista (estrutura list_t a
 * ser definida pelo grupo no ficheiro list-private.h).
 * Retorna a lista ou NULL em caso de erro.
 */
struct list_t *list_create(){

    struct list_t *new_list=(struct list_t *)malloc(sizeof(struct list_t));
    if(new_list==NULL)
        return NULL;
    
    new_list->head=NULL;
    new_list->size=0;

    return new_list;

}

/* Função que adiciona à lista a entry passada como argumento.
 * A entry é inserida de forma ordenada, tendo por base a comparação
 * de entries feita pela função entry_compare do módulo entry e
 * considerando que a entry menor deve ficar na cabeça da lista.
 * Se já existir uma entry igual (com a mesma chave), a entry
 * já existente na lista será substituída pela nova entry,
 * sendo libertada a memória ocupada pela entry antiga.
 * Retorna 0 se a entry ainda não existia, 1 se já existia e foi
 * substituída, ou -1 em caso de erro.
 */
int list_add(struct list_t *l, struct entry_t *entry){

    if(l==NULL || entry==NULL|| entry->value==NULL || entry->key==NULL)
        return -1;
    //adicionar a entry o inicio
    
    if(l->head==NULL || entry_compare(l->head->entry,entry)>0){

        

        struct node_t *new_node= (struct node_t *) malloc(sizeof(struct node_t));
        new_node->entry=entry;
        new_node->next = l->head; 
        l->head=new_node;
        l->size= l->size+1;

        return 0;

    }else if(entry_compare(l->head->entry,entry)==0){ //alterar a entry do inicio

        entry_destroy(l->head->entry);
        l->head->entry=entry;

        //entry_replace(current_node->next->entry,entry->key,entry->value);
        return 1;
    }
    else{//adicionar a entry no meio/fim

        struct node_t *current_node = l->head;
        
        if (current_node->next!=NULL){

            int valor = entry_compare(current_node->next->entry,entry);
            
            while (current_node->next!=NULL && valor<0){
               
                current_node = current_node->next; 
                valor = current_node->next == NULL?valor:entry_compare(current_node->next->entry,entry);
    
            }

            if(valor==0){

                entry_destroy(current_node->next->entry);
                current_node->next->entry=entry;
                return 1;
            }

        }
     
        struct node_t *new_node= (struct node_t *) malloc(sizeof(struct node_t));
        new_node->entry=entry;
        new_node->next=current_node->next;
		current_node->next=new_node;
    }

    l->size++;
    return 0;        

}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *l){
    if(l==NULL)
        return -1;
        
    return l->size;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
*/
struct entry_t *list_get(struct list_t *l, char *key){

    if(l==NULL || l->size==0 || key==NULL)
        return NULL;
    
    struct node_t *current_node = l->head;

    while(current_node!=NULL){

        if(strcmp(current_node->entry->key,key)==0){
            return current_node->entry;
        }
        current_node=current_node->next;
    }

    return NULL;

}

/* Função auxiliar que constrói um array de char* com a cópia de todas as keys na 
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *l){

    if(l==NULL || l->head==NULL)
        return NULL;

    char **keys_array = (char **)malloc((l->size + 1) * sizeof(char *));
    if (keys_array == NULL) {
        return NULL;
    }


    struct node_t *current_node = l->head;

    int i;
    for (i = 0; i < l->size; i++) {

        if (current_node == NULL || current_node->entry == NULL) {
            for (int j = 0; j < i; j++) {
                free(keys_array[j]);
            }
            free(keys_array);
            return NULL;
        }
        
        keys_array[i] = strdup(current_node->entry->key);
        if (keys_array[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(keys_array[j]);
            }
            free(keys_array);
            return NULL;
        }
        current_node = current_node->next;
    }
    
    keys_array[i] = NULL;
    return keys_array;


}

/* Função auxiliar que liberta a memória ocupada pelo array de keys obtido pela 
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys){
    if(keys == NULL){
        return -1;
    }

    for (int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
    }

    free(keys);
    return 0;
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *l, char *key){
    if (key == NULL || l == NULL || l->size < 0){
        return -1;
    }

    struct node_t *current_node = l->head;
    struct node_t *next;

    if(current_node == NULL){
        return 1;
        }
    if(strcmp( l->head->entry->key,key)==0){
            next= l->head->next;
            entry_destroy( l->head->entry);
            l->head=next;
            l->size--;
            return 0;
    }

    while(current_node ->next!=NULL){
        
        if(strcmp(current_node->next->entry->key,key)==0){

            next=current_node->next->next;
            entry_destroy(current_node->next->entry);
            current_node->next=next;
            l->size--;
            return 0;
        }
        current_node=current_node->next;
    }

    return 1;
}

/* Função que elimina uma lista, libertando *toda* a memória utilizada
 * pela lista (incluindo todas as suas entradas).
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_destroy(struct list_t *l){


    if(l==NULL)
        return -1;
    
    struct node_t *current = l->head;
    struct node_t *next;

    
    while (current != NULL) {
        next = current->next;
        entry_destroy(current->entry); 
        free(current); 
        current = next;
    }
    
    
    free(l);
    return 0;
}


