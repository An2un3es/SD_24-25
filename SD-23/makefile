# Definindo variáveis
CC = gcc
CFLAGS = -Iinclude -Wall
DEBUG = 0

ifeq ($(DEBUG), 1)
    CFLAGS += -g
else
    CFLAGS += -g0
endif

# Definindo diretórios
SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary
LIB_DIR = lib
INCLUDE_DIR = include

# Garantir que as pastas de destino existam
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR))

# Ficheiros de objeto comuns
OBJECTS = $(OBJ_DIR)/block.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/htmessages.pb-c.o
CLIENT_OBJECTS = $(OBJ_DIR)/client_hashtable.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/client_network.o $(OBJ_DIR)/message.o
SERVER_OBJECTS = $(OBJ_DIR)/server_hashtable.o $(OBJ_DIR)/server_network.o $(OBJ_DIR)/server_skeleton.o $(OBJ_DIR)/message.o

# Alvo principal
all: libtable client_hashtable server_hashtable

# Criar a biblioteca libtable.a e movê-la para o diretório lib
libtable: $(OBJECTS)
	ar -rcs $(LIB_DIR)/libtable.a $(OBJECTS)

# Compilando o cliente
client_hashtable: $(CLIENT_OBJECTS) lib/libtable.a
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client_hashtable $(CLIENT_OBJECTS) lib/libtable.a -lprotobuf-c

# Compilando o servidor
server_hashtable: $(SERVER_OBJECTS) lib/libtable.a
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server_hashtable $(SERVER_OBJECTS) lib/libtable.a -lprotobuf-c

# Regras para compilar o arquivo gerado a partir do htmessages.proto
$(OBJ_DIR)/htmessages.pb-c.o: $(SRC_DIR)/htmessages.pb-c.c
	$(CC) $(CFLAGS) -c $< -o $@

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

# Regra para compilar o ficheiro message.c
$(OBJ_DIR)/message.o: $(SRC_DIR)/message.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar os arquivos objeto comuns
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* $(LIB_DIR)/*

.PHONY: all clean
# Definindo variáveis
CC = gcc
CFLAGS = -Iinclude -Wall
DEBUG = 0

ifeq ($(DEBUG), 1)
    CFLAGS += -g
else
    CFLAGS += -g0
endif

# Definindo diretórios
SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary
LIB_DIR = lib
INCLUDE_DIR = include

# Garantir que as pastas de destino existam
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR))

# Definindo variáveis para a biblioteca ZooKeeper
ZK_INC = -I/usr/include/zookeeper
ZK_LIB = -L/usr/lib -lzookeeper_mt

# Atualizando CFLAGS para incluir os cabeçalhos do ZooKeeper
CFLAGS += $(ZK_INC)

# Ficheiros de objeto comuns
OBJECTS = $(OBJ_DIR)/block.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/htmessages.pb-c.o
CLIENT_OBJECTS = $(OBJ_DIR)/client_hashtable.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/client_network.o $(OBJ_DIR)/message.o $(OBJ_DIR)/zookeeper_usage.o
SERVER_OBJECTS = $(OBJ_DIR)/server_hashtable.o $(OBJ_DIR)/server_network.o $(OBJ_DIR)/server_skeleton.o $(OBJ_DIR)/message.o $(OBJ_DIR)/zookeeper_usage.o

# Alvo principal
all: libtable client_hashtable server_hashtable

# Criar a biblioteca libtable.a e movê-la para o diretório lib
libtable: $(OBJECTS)
	ar -rcs $(LIB_DIR)/libtable.a $(OBJECTS)

# Compilando o cliente
client_hashtable: $(CLIENT_OBJECTS) lib/libtable.a
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client_hashtable $(CLIENT_OBJECTS) lib/libtable.a -lprotobuf-c $(ZK_LIB)

# Compilando o servidor
server_hashtable: $(SERVER_OBJECTS) lib/libtable.a
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server_hashtable $(SERVER_OBJECTS) lib/libtable.a -lprotobuf-c $(ZK_LIB)

# Regras para compilar o arquivo gerado a partir do htmessages.proto
$(OBJ_DIR)/htmessages.pb-c.o: $(SRC_DIR)/htmessages.pb-c.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar os arquivos objeto do cliente
$(OBJ_DIR)/client_hashtable.o: $(SRC_DIR)/client_hashtable.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client_stub.o: $(SRC_DIR)/client_stub.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client_network.o: $(SRC_DIR)/client_network.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar o zookeeper_usage.c (dependente de client_stub.o e server_skeleton.o)
$(OBJ_DIR)/zookeeper_usage.o: $(SRC_DIR)/zookeeper_usage.c $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/server_skeleton.o
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar os arquivos objeto do servidor
$(OBJ_DIR)/server_hashtable.o: $(SRC_DIR)/server_hashtable.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server_network.o: $(SRC_DIR)/server_network.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server_skeleton.o: $(SRC_DIR)/server_skeleton.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar o ficheiro message.c
$(OBJ_DIR)/message.o: $(SRC_DIR)/message.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras para compilar os arquivos objeto comuns
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* $(LIB_DIR)/*

.PHONY: all clean
