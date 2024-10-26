
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "htmessages.pb-c.h"
#include "message-private.h"
#include "server_skeleton.h"

/* Função para preparar um socket de receção de pedidos de ligação
* num determinado porto.
* Retorna o descritor do socket ou -1 em caso de erro.
*/
int server_network_init(short port){

    int sockfd;
    struct sockaddr_in server;

    // Cria o socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erro ao criar o socket");
        return -1;
    }

    // Preenche a estrutura server para o bind
    memset(&server, 0, sizeof(server)); 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(port); 

    // Faz o bind do socket ao endereço e à porta
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Erro ao fazer o bind");
        close(sockfd); // Fecha o socket em caso de erro
        return -1;
    }

    // Coloca o socket em modo de escuta para aceitar conexões
    if (listen(sockfd, 5) < 0) {
        printf("Erro ao executar o listen");
        close(sockfd); // Fecha o socket em caso de erro
        return -1;
    }

    // Retorna o descritor do socket pronto para aceitar conexões
    return sockfd;
}

/* A função network_receive() deve:
* - Ler os bytes da rede, a partir do client_socket indicado;
* - De-serializar estes bytes e construir a mensagem com o pedido,
* reservando a memória necessária para a estrutura MessageT.
* Retorna a mensagem com o pedido ou NULL em caso de erro.
*/
MessageT *network_receive(int client_socket) {
    uint8_t buffer[1024];  // Buffer grande o suficiente
    int recv_size = sizeof(buffer); // Tamanho do buffer para ler

    // Ler os dados do socket
    if (read_all(client_socket, buffer, recv_size) < 0) {
        printf("Erro ao receber dados");
        return NULL;
    }

    // Deserializar a mensagem recebida usando Protocol Buffers
    MessageT *message = message_t__unpack(NULL, recv_size, buffer);
    if (message == NULL) {
        fprintf(stderr, "Erro ao de-serializar a mensagem\n");
        return NULL;
    }

    return message;
}


/* A função network_send() deve:
* - Serializar a mensagem de resposta contida em msg;
* - Enviar a mensagem serializada, através do client_socket.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int network_send(int client_socket, MessageT *msg) {
    // Serializar a mensagem usando Protocol Buffers
    int size = message_t__get_packed_size(msg);
    uint8_t *buffer = malloc(size);

    if (buffer == NULL) {
        printf("Erro de alocação de memória");
        return -1;
    }

    message_t__pack(msg, buffer);

    // Enviar a mensagem serializada usando write_all
    if (write_all(client_socket, buffer, size) < 0) {
        printf("Erro ao enviar dados");
        free(buffer);
        return -1;
    }
    free(buffer);
    return 0;
}

/* A função network_main_loop() deve:
* - Aceitar uma conexão de um cliente; 
* - Receber uma mensagem usando a função network_receive;
* - Entregar a mensagem de-serializada ao skeleton para ser processada
na tabela table;
* - Esperar a resposta do skeleton;
* - Enviar a resposta ao cliente usando a função network_send.
* A função não deve retornar, a menos que ocorra algum erro. Nesse
* caso retorna -1.
*/
int network_main_loop(int listening_socket, struct table_t *table){

    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_socket;

    // Aceitar a conexão de um cliente
    while ((client_socket = accept(listening_socket, (struct sockaddr *)&client, &client_len)) > 0) {
        printf("Cliente conectado.\n");

        // Receber a mensagem do cliente

        MessageT *request_msg = network_receive(client_socket);

        if (request_msg == NULL) { 
            printf("Erro ao receber a mensagem do cliente.\n");
            MessageT error_msg;
            message_t__init(&error_msg);
            error_msg.opcode=MESSAGE_T__OPCODE__OP_ERROR;
            error_msg.c_type=MESSAGE_T__C_TYPE__CT_NONE;
            network_send(client_socket, &error_msg);
            close(client_socket);
            continue;
        }

        // Processar a mensagem no skeleton
        if (invoke(request_msg, table)<0) {
            printf("Erro ao processar a mensagem.\n");
            MessageT error_msg;
            message_t__init(&error_msg);
            error_msg.opcode=MESSAGE_T__OPCODE__OP_ERROR;
            error_msg.c_type=MESSAGE_T__C_TYPE__CT_NONE;
            network_send(client_socket, &error_msg);
            close(client_socket);

            continue;
        }

        // Enviar a resposta de volta ao cliente
        if (network_send(client_socket, request_msg) < 0) {
            printf("Erro ao enviar a resposta para o cliente.\n");
            close(client_socket);
            continue;
        }

        //Limpar e fechar a conexão
        message_t__free_unpacked(request_msg, NULL);
        close(client_socket);
    }

    return -1;

}


/* Liberta os recursos alocados por server_network_init(), nomeadamente
* fechando o socket passado como argumento.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int server_network_close(int socket){
    printf("Desconectando...");
    return (socket <0 || close(socket)<0)? -1:0 ; //mais  simples
}

