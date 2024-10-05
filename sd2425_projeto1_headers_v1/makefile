# Definindo variáveis
CC = gcc
CFLAGS = -Iinclude -Wall -g
SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary

# Listando os arquivos fonte e de teste
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Nome do executável
TARGET = $(BIN_DIR)/programa

# Regra principal
all: $(TARGET)

# Regra para criar o executável principal
$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $^

# Regra para compilar os arquivos .c em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar todos os testes
tests: $(OBJ_FILES) $(OBJ_DIR)/test_block.o $(OBJ_DIR)/test_entry.o $(OBJ_DIR)/test_list.o $(OBJ_DIR)/test_table.o $(OBJ_DIR)/test_serialization.o
	$(CC) -o $(BIN_DIR)/test_programa $(OBJ_DIR)/test_block.o $(OBJ_DIR)/test_entry.o $(OBJ_DIR)/test_list.o $(OBJ_DIR)/test_table.o $(OBJ_DIR)/test_serialization.o $(OBJ_FILES)

# Regra para compilar os arquivos de teste
$(OBJ_DIR)/test_%.o: $(SRC_DIR)/test_%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET) $(BIN_DIR)/test_programa

.PHONY: all clean tests