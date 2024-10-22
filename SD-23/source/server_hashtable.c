#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htmessages.pb-c.h"

/*
Até agora foi só feita parte da conexão como o professor explicou na aula TP
Falta a parte de criar a hashtable consoante o argumento <n_lists> antes de permitir a conexão de clients ao server.
*/


int main(int argc, char **argv) {

    

    int sockfd, connsockfd;
    struct sockaddr_in server, client;
    socklen_t size_client = sizeof(struct sockaddr_in);

    // Verifica se foi passado algum argumento
    if (argc != 3) {
        printf("Uso: %s <server>:<port> <n_lists> \n", argv[0]);
        return -1;
    }

    //criar tabela com n_listas pedidas
    int n_listas = atoi(argv[2]);
    struct table_t *table =server_skeleton_init(n_listas);

    if(table==NULL){
        perror("Erro ao criar a tabela");
        return -1;
    }



    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    // Preenche estrutura server para bind
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1])); // <port> é a porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY);

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