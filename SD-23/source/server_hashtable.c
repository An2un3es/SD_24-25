/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htmessages.pb-c.h"
#include "server_network.h"
#include "server_skeleton.h"
#include "client_stub.h"
#include "zookeeper_usage.h"

#define ZDATALEN 1024 * 1024

static char *watcher_ctx = "ZooKeeper Data Watcher";



int listening_socket;
void closeServer(){
    server_network_close(listening_socket);
    exit(0);
}
int main(int argc, char **argv) {

    signal(SIGPIPE, SIG_IGN);  // Ignorar SIGPIPE
    signal(SIGINT, closeServer);  // Ignorar SIGPIPE

    // Verifica se foi passado algum argumento
    if (argc != 4) {
        printf("Erro ao iniciar servidor\n");
        printf("Exemplo de uso: %s <zookeeper_ip:zookeeper_port> <server>:<port> <n_lists> \n", argv[0]);
        return -1;
    }


    // Extrair <server> e <port> do argv[1]
    char *zoo_server_and_port= argv[1];
    char *zoo_ip = strtok(zoo_server_and_port, ":");
    char *zoo_port_str = strtok(NULL, ":");

    if (zoo_ip == NULL || zoo_port_str == NULL) {
        fprintf(stderr, "Erro: O formato esperado é <Zoo_ip>:<Zoo_port>\n");
        return -1;
    }

    int zoo_port = atoi(zoo_port_str);

    char *server_and_port = argv[2];
    char *server_ip = strtok(server_and_port, ":");
    char *port_str = strtok(NULL, ":");

    if (server_ip == NULL || port_str == NULL) {
        fprintf(stderr, "Erro: O formato esperado é <server>:<port>\n");
        return -1;
    }

    int port = atoi(port_str);
    int n_listas = atoi(argv[3]);
    struct server_t* server =server_init(n_listas,zoo_server_and_port);

    if(server==NULL){
        printf("Erro ao iniciar servidor");
        fflush(stdout);
        return -1;
    }


    if(server->table==NULL){
        printf("Erro ao criar a tabela");
        fflush(stdout);
        return -1;
    }

    // Inicializar o servidor (criar socket de escuta)
    listening_socket = server_network_init(port);
    if (listening_socket < 0) {
        printf("Erro ao inicializar a rede do servidor");
        fflush(stdout);
        return -1;
    }


    // Entrar no loop principal para processar as conexões
    if (network_main_loop(listening_socket, server->table) < 0) {
        printf("Erro no loop de rede");
        close(listening_socket);
        return -1;
    }

    // Encerrar o servidor
    server_destroy(server);

    return 0;
}