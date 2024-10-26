# Definindo variáveis
CC = gcc
CFLAGS = -Iinclude -Wall
DEBUG = 0

ifeq ($(DEBUG), 1)
    CFLAGS += -g
else
    CFLAGS += -g0
endif

SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary

# Garantir que as pastas de destino existam
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Alvos para os arquivos de objeto
OBJECTS = $(OBJ_DIR)/block.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
CLIENT_OBJECTS = $(OBJ_DIR)/client_hashtable.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/client_network.o
SERVER_OBJECTS = $(OBJ_DIR)/server_hashtable.o $(OBJ_DIR)/server_network.o $(OBJ_DIR)/server_skeleton.o

# Alvo principal
all: libtable client_hashtable server_hashtable

# Criar a biblioteca libtable.a
libtable: $(OBJECTS)
	ar -rcs libtable.a $(OBJECTS)

# Compilando o cliente
client_hashtable: $(CLIENT_OBJECTS) libtable.a
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client_hashtable $(CLIENT_OBJECTS) libtable.a

# Compilando o servidor
server_hashtable: $(SERVER_OBJECTS) libtable.a
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server_hashtable $(SERVER_OBJECTS) libtable.a

# Regras para compilar os arquivos objeto do cliente
$(OBJ_DIR)/client_hashtable.o: $(SRC_DIR)/client_hashtable.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client_stub.o: $(SRC_DIR)/client_stub.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client_network.o: $(SRC_DIR)/client_network.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar os arquivos objeto do servidor
$(OBJ_DIR)/server_hashtable.o: $(SRC_DIR)/server_hashtable.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server_network.o: $(SRC_DIR)/server_network.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server_skeleton.o: $(SRC_DIR)/server_skeleton.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar os arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* libtable.a

.PHONY: all clean
