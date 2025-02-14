/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"

/* Função que cria um novo bloco de dados block_t e que inicializa
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct block_t* block_create(int size, void* data)
{

    if (size <= 0 || data == NULL)
    {
        return NULL;
    }

    struct block_t* b = (struct block_t*)malloc(sizeof(struct block_t));

    if (b == NULL)
    {
        return NULL;
    }

    b->datasize = size;
    b->data = data;

    return b;
}

/* Função que duplica uma estrutura block_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct block_t* block_duplicate(struct block_t* b)
{

    if (b == NULL || b->datasize <= 0 || b->data == NULL)
    {
        return NULL;
    }
    struct block_t* new_b = (struct block_t*)malloc(sizeof(struct block_t));
    if (new_b == NULL)
    {
        return NULL;
    }
    new_b->datasize = b->datasize;
    new_b->data = malloc(b->datasize);
    if (new_b->data == NULL)
    {
        free(new_b);
        return NULL;
    }
    memcpy(new_b->data, b->data, b->datasize);
    return new_b;
}

/* Função que substitui o conteúdo de um bloco de dados block_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int block_replace(struct block_t* b, int new_size, void* new_data)
{
    if (b == NULL || new_data == NULL || new_size <= 0) { // Verficar se valido
        return -1;
    }

    if (b->data == new_data) {

    }
    else if (b->data != NULL) { // se tiver algo dentro apaga
        free(b->data);
        b->data = malloc(new_size);
        if (b->data == NULL) {
            return -1;
        }
        memcpy(b->data, new_data, new_size);
    }else 
        return -1;
    b->datasize = new_size;
    return 0;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro b,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int block_destroy(struct block_t* b)
{
    if (b == NULL)
        return -1;

    if (b->data != NULL)
    {
        free(b->data);
        b->data = NULL;
    } 
    free(b);
    b = NULL;
    return 0;
}
