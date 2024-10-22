#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htmessages.pb-c.h"

int main(int argc, char **argv) {


    // Verifica se foi passado algum argumento
    if (argc != 3) {
        printf("Erro ao iniciar servidor\n");
        printf("Exemplo de uso: %s <server>:<port> <n_lists> \n", argv[0]);
        return -1;
    }

    // Extrair <server> e <port> do argv[1]
    char *server_and_port = argv[1];
    char *server_ip = strtok(server_and_port, ":");
    char *port_str = strtok(NULL, ":");

    if (server_ip == NULL || port_str == NULL) {
        fprintf(stderr, "Erro: O formato esperado é <server>:<port>\n");
        return -1;
    }

    int port = atoi(port_str);


    //criar tabela com n_listas pedidas
    int n_listas = atoi(argv[2]);
    struct table_t *table =server_skeleton_init(n_listas);

    if(table==NULL){
        perror("Erro ao criar a tabela");
        return -1;
    }


    // Cria socket TCP
    int sockfd, connsockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket");
        return -1;
    }


    // Preenche estrutura server para bind
    struct sockaddr_in server, client;
    socklen_t size_client = sizeof(struct sockaddr_in);

    server.sin_family = AF_INET;
    server.sin_port = htons(port); // <port> é a porta TCP

    // Converter e atribuir o endereço IP (server)
    if (inet_pton(AF_INET, server_ip, &server.sin_addr) <= 0) {
        perror("Erro ao converter o endereço IP");
        return -1;
    }

    // Faz bind
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    // Faz listen
    if (listen(sockfd, 5) < 0) {
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }

    printf("Servidor à espera de conexões\n");

    // Bloqueia à espera de pedidos de conexão
    while ((connsockfd = accept(sockfd, (struct sockaddr *)&client, &size_client)) != -1) {
        printf("Cliente conectado\n");



    }
}