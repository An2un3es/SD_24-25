#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "table.h"
#include "block.h"

/**************************************************************/
void pee(const char *msg)
{
	perror(msg);
	exit(0);
}

/**************************************************************/
int testEmptyTable() {
	struct table_t *table = table_create(5);

	printf("Módulo table -> testEmptyTable: ");
	fflush(stdout);

	int result = (table != NULL) && (table_size(table) == 0);

	table_destroy(table);
	
	printf("%s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
int testPutInexistente() {
	int result, i;
	struct table_t *table = table_create(5);
	char *key[1024];
	struct block_t *blocks[1024], *b;

	printf("Módulo table -> testPutInexistente: ");
	fflush(stdout);

	for(i=0; i<1024; i++) {
		key[i] = (char*)malloc(16*sizeof(char));
		sprintf(key[i],"a/key/b-%d",i);
		blocks[i] = block_create(strlen(key[i])+1, strdup(key[i]));

		table_put(table, key[i], blocks[i]);
	}

	assert(table_size(table) == 1024);
	result = (table_size(table) == 1024);

	for(i=0; i<1024; i++) {
		b = table_get(table, key[i]);

		assert(b->datasize == blocks[i]->datasize);
		assert(memcmp(b->data, blocks[i]->data, b->datasize) == 0);
		assert(b->data != blocks[i]->data);

		result = result && (b->datasize == blocks[i]->datasize
                                && memcmp(b->data, blocks[i]->data, b->datasize) == 0
                                && b->data != blocks[i]->data);
		block_destroy(b);
	}

	for(i=0; i<1024; i++) {
		free(key[i]);
		block_destroy(blocks[i]);
	}

	table_destroy(table);
	
	printf("%s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
int testPutExistente() {
	int result, i;
	struct table_t *table = table_create(5);
	char *key[1024];
	struct block_t *blocks[1024], *b;

	printf("Módulo table -> testPutExistente: ");
	fflush(stdout);

	for(i=0; i<1024; i++) {
		key[i] = (char*)malloc(16*sizeof(char));
		sprintf(key[i], "a/key/b-%d", i);
		blocks[i] = block_create(strlen(key[i])+1, strdup(key[i]));

		table_put(table, key[i], blocks[i]);
	}

	assert(table_size(table) == 1024);
	result = (table_size(table) == 1024);

	b = block_create(strlen("256")+1, strdup("256"));
	table_put(table, key[256], b);
	block_destroy(b);

	assert(table_size(table) == 1024);
	result = result && (table_size(table) == 1024);

	for(i=0; i<1024; i++) {
		b = table_get(table, key[i]);
		
		if(i==256) {
			result = result && (b->datasize == strlen("256")+1 && 
        	                   memcmp(b->data, "256", b->datasize) == 0);
		} else {
			result = result && (b->datasize == blocks[i]->datasize && 
        	                   memcmp(b->data,blocks[i]->data,b->datasize) == 0 &&
        	                   b->data != blocks[i]->data);
		}

		block_destroy(b);
	}

	for(i=0; i<1024; i++) {
		free(key[i]);
		block_destroy(blocks[i]);
	}

	table_destroy(table);
	
	printf("%s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
int testDelInexistente() {
	int result, i;
	struct table_t *table = table_create(7);
	char *key;
	struct block_t *b;

	printf("Módulo table -> testDelInexistente: ");
	fflush(stdout);

	for(i=0; i<1024; i++) {
		key = (char*)malloc(16*sizeof(char));
		sprintf(key, "a/key/b-%d", i);
		b = block_create(strlen(key)+1, key);

		table_put(table, key, b);

		block_destroy(b);
	}

	assert(table_size(table) == 1024);
	result = (table_size(table) == 1024);

	result = result && (table_get(table, "a/key/b-1024") == NULL) &&
			   (table_get(table, "abc") == NULL);

	result = result && (table_remove(table, "a/key/b-1024") == 1) &&
			   (table_remove(table, "abc") == 1);

	result = result && (table_get(table, "a/key/b-1024") == NULL) &&
			   (table_get(table, "abc") == NULL);

	assert(table_size(table) == 1024);
	result = result && (table_size(table) == 1024);

	table_destroy(table);
	
	printf("%s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
int testDelExistente() {
	int result, i;
	struct table_t *table = table_create(7);
	char *key;
	struct block_t *b, *b2;

	printf("Módulo table -> testDelExistente: ");
	fflush(stdout);

	for(i=0; i<1024; i++) {
		key = (char*)malloc(16*sizeof(char));
		sprintf(key, "a/key/b-%d", i);
		b = block_create(strlen(key)+1, key);

		table_put(table, key, b);

		block_destroy(b);
	}

	assert(table_size(table) == 1024);
	result = (table_size(table) == 1024);

	result = result && ((b = table_get(table,"a/key/b-1023")) != NULL) &&
			   ((b2 = table_get(table,"a/key/b-45")) != NULL);

	block_destroy(b);
	block_destroy(b2);

	result = result && (table_remove(table,"a/key/b-1023") == 0) &&
			   (table_remove(table,"a/key/b-45") == 0);

	result = result && (table_get(table,"a/key/b-1023") == NULL) &&
			   (table_get(table,"a/key/b-45") == NULL);

	result = result && (table_remove(table,"a/key/b-1023") == 1) &&
			   (table_remove(table,"a/key/b-45") == 1);

	assert(table_size(table) == 1022);
	result = result && (table_size(table) == 1022);

	table_destroy(table);
	
	printf("%s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
int testGetKeys() {
	int result = 1, i, j, achou;
	struct table_t *table = table_create(2);
	char **keys;
	char *k[4] = {"abc","bcd","cde","def"};
	struct block_t *b = block_create(3, strdup("123"));

	printf("Módulo table -> testGetKeys: ");
	fflush(stdout);

	table_put(table, k[3], b);
	table_put(table, k[2], b);
	table_put(table, k[1], b);
	table_put(table, k[0], b);

	block_destroy(b);

	keys = table_get_keys(table);
	
	for(i=0; keys[i] != NULL; i++) {
		achou = 0;
		for(j=0; j<4; j++) {
			achou = (achou || (strcmp(keys[i], k[j]) == 0));
		}
		result = (result && achou);
	}

	result = result && (table_size(table) == i);

	table_free_keys(keys);

	table_destroy(table);

	printf("%s\n",result?"passou":"não passou");
	return result;
}

int testInsertMaxSize() {
    struct table_t *table = table_create(5);
    char *key = (char *)malloc(16 * sizeof(char));
    struct block_t *block;

    printf("Módulo table -> testInsertMaxSize: ");
    fflush(stdout);

    for (int i = 0; i < 5; i++) {
        sprintf(key, "key-%d", i);
        block = block_create(strlen(key) + 1, strdup(key));
        table_put(table, key, block);
        block_destroy(block);
    }

    // Testa inserção quando a tabela está cheia
    sprintf(key, "key-overflow");
    block = block_create(strlen(key) + 1, strdup(key));
    int result = table_put(table, key, block);  // Espera-se que retorne algum erro
    block_destroy(block);

    assert(result == 0); // Verifica se a inserção falhou
    printf("passou\n");

    free(key);
    table_destroy(table);
    return 1;
}

/**************************************************************/
int testRemoveFromEmptyTable() {
    struct table_t *table = table_create(5);
    printf("Módulo table -> testRemoveFromEmptyTable: ");
    fflush(stdout);

    int result = (table_remove(table, "nonexistent") == 1);
    printf("%s\n", result ? "passou" : "não passou");

    table_destroy(table);
    return result;
}

/**************************************************************/
int testInsertDuplicateKeys() {
    struct table_t *table = table_create(5);
    char *key = (char *)malloc(16 * sizeof(char));
    struct block_t *block1, *block2;

    printf("Módulo table -> testInsertDuplicateKeys: ");
    fflush(stdout);

    sprintf(key, "duplicate-key");
    block1 = block_create(strlen(key) + 1, strdup("value1"));
    table_put(table, key, block1);

    block2 = block_create(strlen(key) + 1, strdup("value2"));
    table_put(table, key, block2);  // Insere chave duplicada

    struct block_t *retrieved = table_get(table, key);
    int result = (strcmp(retrieved->data, "value2") == 0);  // Espera-se que o valor seja "value2"

    block_destroy(retrieved);
    printf("%s\n", result ? "passou" : "não passou");

    block_destroy(block1);
    block_destroy(block2);
    free(key);
    table_destroy(table);
    return result;
}

/**************************************************************/
int testRemoveAllElements() {
    struct table_t *table = table_create(10);
    char *key = (char *)malloc(16 * sizeof(char));
    int result = 1;

    printf("Módulo table -> testRemoveAllElements: ");
    fflush(stdout);

    for (int i = 0; i < 10; i++) {
        sprintf(key, "key-%d", i);
        struct block_t *block = block_create(strlen(key) + 1, strdup(key));
        table_put(table, key, block);
        block_destroy(block);
    }

    // Remove todos os elementos
    for (int i = 0; i < 10; i++) {
        sprintf(key, "key-%d", i);
        result = result && (table_remove(table, key) == 0);  // Espera-se que retorne 0
    }

    result = result && (table_size(table) == 0);  // A tabela deve estar vazia
    printf("%s\n", result ? "passou" : "não passou");

    free(key);
    table_destroy(table);
    return result;
}


/**************************************************************/
int testRetrieveNonexistentKey() {
    struct table_t *table = table_create(5);
    printf("Módulo table -> testRetrieveNonexistentKey: ");
    fflush(stdout);

    struct block_t *result = table_get(table, "nonexistent");
    int test_result = (result == NULL);  // Espera-se que o resultado seja NULL
    printf("%s\n", test_result ? "passou" : "não passou");

    table_destroy(table);
    return test_result;
}

/**************************************************************/
int main() {
    int score = 0;

    printf("\nIniciando teste do módulo table \n");

    // Testes existentes
    score += testEmptyTable();
    score += testPutInexistente();
    score += testPutExistente();
    score += testDelInexistente();
    score += testDelExistente();
    score += testGetKeys();

    // Novos testes complementares
    score += testInsertMaxSize();
    score += testRemoveFromEmptyTable();
    score += testInsertDuplicateKeys();
    score += testRemoveAllElements();
    score += testRetrieveNonexistentKey();

    printf("teste table (score): %d/11\n", score);

    if (score == 8)
        return 0;
    else
        return -1;
}