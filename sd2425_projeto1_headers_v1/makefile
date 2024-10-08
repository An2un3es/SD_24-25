# Definindo variáveis
CC = gcc
CFLAGS = -Iinclude -Wall -g0
SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary

# Garantir que as pastas de destino existam
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Definindo os arquivos de objeto
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))

# Alvo principal
all: tests

# Teste de block
$(BIN_DIR)/test_block: $(SRC_DIR)/test_block.c $(OBJ_DIR)/block.o
	$(CC) $(CFLAGS) $(SRC_DIR)/test_block.c $(OBJ_DIR)/block.o -o $(BIN_DIR)/test_block

# Teste de entry
$(BIN_DIR)/test_entry: $(SRC_DIR)/test_entry.c $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o
	$(CC) $(CFLAGS) $(SRC_DIR)/test_entry.c $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o -o $(BIN_DIR)/test_entry

# Teste de list
$(BIN_DIR)/test_list: $(SRC_DIR)/test_list.c $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o
	$(CC) $(CFLAGS) $(SRC_DIR)/test_list.c $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o -o $(BIN_DIR)/test_list

# Teste de table
$(BIN_DIR)/test_table: $(SRC_DIR)/test_table.c $(OBJ_DIR)/table.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o
	$(CC) $(CFLAGS) $(SRC_DIR)/test_table.c $(OBJ_DIR)/table.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o -o $(BIN_DIR)/test_table

# Teste de serialization
$(BIN_DIR)/test_serialization: $(SRC_DIR)/test_serialization.c $(OBJ_DIR)/serialization.o $(OBJ_DIR)/table.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o
	$(CC) $(CFLAGS) $(SRC_DIR)/test_serialization.c $(OBJ_DIR)/serialization.o $(OBJ_DIR)/table.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/block.o -o $(BIN_DIR)/test_serialization

# Regra para compilar todos os testes de uma vez
tests: $(BIN_DIR)/test_block $(BIN_DIR)/test_entry $(BIN_DIR)/test_list $(BIN_DIR)/test_table $(BIN_DIR)/test_serialization

# Compilando os arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/*

.PHONY: all tests clean
