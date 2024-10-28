#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "client_stub.h"
#include "client_stub-private.h"
#include "htmessages.pb-c.h"
#include "message-private.h"  


#define BUFFER_SIZE 1024 
/* Esta função deve:
* - Obter o endereço do servidor (struct sockaddr_in) com base na
* informação guardada na estrutura rtable;
* - Estabelecer a ligação com o servidor;
* - Guardar toda a informação necessária (e.g., descritor do socket)
* na estrutura rtable;
* - Retornar 0 (OK) ou -1 (erro).
*/
int network_connect(struct rtable_t *rtable){

    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Error creating TCP socket");
      return -1;
    }

    rtable->sockfd=sockfd;

    // Preenche a estrutura server
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);


    // Converte o endereço IP
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(rtable-> sockfd);
        return -1;
    }

    // Estabelece a conexão
    if (connect(rtable-> sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(rtable-> sockfd);
        return -1;
    }

    printf("network_connect FUNCIONOU\n");
    return 0;

}

/* Esta função deve:
* - Obter o descritor da ligação (socket) da estrutura rtable_t;
* - Serializar a mensagem contida em msg;
* - Enviar a mensagem serializada para o servidor;
* - Esperar a resposta do servidor;
* - De-serializar a mensagem de resposta;   
* - Tratar de forma apropriada erros de comunicação;
* - Retornar a mensagem de-serializada ou NULL em caso de erro.
*/
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if (rtable == NULL || msg == NULL) {
        fprintf(stderr, "Erro: %s é NULL.\n", rtable == NULL ? "rtable" : "msg");
        return NULL;
    }

    // Obter o descritor da ligação (socket) da estrutura rtable_t
    int socketfd = rtable->sockfd;
    if (socketfd <= 0) {
        fprintf(stderr, "Erro: descritor de socket inválido.\n");
        return NULL;
    }

    // Serializar a mensagem contida em msg
    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *msg_serialized = (uint8_t *)malloc(msg_size);
    if (msg_serialized == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória para a mensagem serializada.\n");
        return NULL;
    }
    message_t__pack(msg, msg_serialized);

    // Enviar o tamanho da mensagem ao servidor
    uint32_t msg_size_network = htonl(msg_size);  // Converter para ordem de rede
    if (write_all(socketfd, &msg_size_network, sizeof(msg_size_network)) < 0) {
        fprintf(stderr, "Erro ao enviar o tamanho da mensagem ao servidor.\n");
        free(msg_serialized);
        return NULL;
    }

    // Enviar a mensagem serializada para o servidor
    if (write_all(socketfd, msg_serialized, msg_size) < 0) {
        fprintf(stderr, "Erro ao enviar a mensagem ao servidor.\n");
        free(msg_serialized);
        return NULL;
    }
    free(msg_serialized);  

    // Esperar a resposta do servidor (tamanho da mensagem)
    uint32_t response_size_network;
    if (read_all(socketfd, &response_size_network, sizeof(response_size_network)) < 0) {
        fprintf(stderr, "Erro ao receber o tamanho da resposta do servidor.\n");
        return NULL;
    }
    size_t response_size = ntohl(response_size_network);  // Convertendo para ordem do host

    // Alocar memória para a resposta e receber a mensagem completa
    uint8_t *response_buf = (uint8_t *)malloc(response_size);
    if (response_buf == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória para a resposta.\n");
        return NULL;
    }
    if (read_all(socketfd, response_buf, response_size) < 0) {
        fprintf(stderr, "Erro ao receber a resposta do servidor.\n");
        free(response_buf);
        return NULL;
    }

    // De-serializar a mensagem de resposta
    MessageT *response_msg = message_t__unpack(NULL, response_size, response_buf);
    free(response_buf);  // Liberar a memória do buffer da resposta

    if (response_msg == NULL) {
        fprintf(stderr, "Erro ao deserializar a resposta do servidor.\n");
        return NULL;
    }

    return response_msg;
}


/* Fecha a ligação estabelecida por network_connect().
* Retorna 0 (OK) ou -1 (erro).
*/
int network_close(struct rtable_t *rtable){

    // Fecho bem-sucedido
    return close(rtable->sockfd);

}