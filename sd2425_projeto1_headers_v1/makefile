# Definindo variáveis
CC = gcc
CFLAGS = -Iinclude -Wall -g
SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary

# Garantir que as pastas de destino existam
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Teste de block
$(BIN_DIR)/test_block: $(SRC_DIR)/test_block.c $(SRC_DIR)/block.c
	$(CC) $(CFLAGS) $(SRC_DIR)/test_block.c $(SRC_DIR)/block.c -o $(BIN_DIR)/test_block

# Teste de entry
$(BIN_DIR)/test_entry: $(SRC_DIR)/test_entry.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c
	$(CC) $(CFLAGS) $(SRC_DIR)/test_entry.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c -o $(BIN_DIR)/test_entry

# Teste de list
$(BIN_DIR)/test_list: $(SRC_DIR)/test_list.c $(SRC_DIR)/list.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c
	$(CC) $(CFLAGS) $(SRC_DIR)/test_list.c $(SRC_DIR)/list.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c -o $(BIN_DIR)/test_list

# Teste de table
$(BIN_DIR)/test_table: $(SRC_DIR)/test_table.c $(SRC_DIR)/table.c $(SRC_DIR)/list.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c
	$(CC) $(CFLAGS) $(SRC_DIR)/test_table.c $(SRC_DIR)/table.c $(SRC_DIR)/list.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c -o $(BIN_DIR)/test_table

# Teste de serialization
$(BIN_DIR)/test_serialization: $(SRC_DIR)/test_serialization.c $(SRC_DIR)/serialization.c $(SRC_DIR)/table.c $(SRC_DIR)/list.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c
	$(CC) $(CFLAGS) $(SRC_DIR)/test_serialization.c $(SRC_DIR)/serialization.c $(SRC_DIR)/table.c $(SRC_DIR)/list.c $(SRC_DIR)/entry.c $(SRC_DIR)/block.c -o $(BIN_DIR)/test_serialization

# Regra para compilar todos os testes de uma vez
tests: $(BIN_DIR)/test_block $(BIN_DIR)/test_entry $(BIN_DIR)/test_list $(BIN_DIR)/test_table $(BIN_DIR)/test_serialization

# Limpeza dos arquivos gerados
clean:
	rm -f $(BIN_DIR)/test_block $(BIN_DIR)/test_entry $(BIN_DIR)/test_list $(BIN_DIR)/test_table $(BIN_DIR)/test_serialization

.PHONY: tests clean
