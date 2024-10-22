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

    return 0;

  /**

    int reuse = 1;
    //SOL_SOCKET e informar que e uma ligacao tcp
    if (setsockopt(rtable -> sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed");

    if (connect(rtable -> sockfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
      printf("Error connecting to the server\n");
      close(rtable -> sockfd);
      return -1;
    }
    printf("Connected\n");

    return 0;
    */

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
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg){ //trabalhamos com sistema sync!
   //TODO rever o codigo
 
 if (rtable == NULL || msg == NULL) {
        printf(stderr, "Erro: %s é NULL.\n", rtable==NULL?rtable:msg); //Caso precisarmos pros testes  TODO remove
        return NULL;
    }

   // - Obter o descritor da ligação (socket) da estrutura rtable_t;
    int socketfd = rtable->sockfd;
    if (socketfd <= 0) {
        printf(stderr, "Erro: descritor de socket inválido.\n"); //Caso precisarmos pros testes TODO remove
        return NULL;
    }
    // - Serializar a mensagem contida em msg;
    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *msg_serialized = (uint8_t *)malloc(msg_size);
    if (msg_serialized == NULL) {
        printf(stderr, "Erro: falha ao alocar memória para a mensagem serializada.\n"); //Caso precisarmos pros testes TODO remove
        return NULL;
    }
    message_t__pack(msg, msg_serialized);
    // - Enviar a mensagem serializada para o servidor;
    if (send(socketfd, msg_serialized, msg_size, 0) != msg_size) {
        fprintf(stderr, "Erro ao enviar a mensagem ao servidor.\n");
        free(msg_serialized);
        return NULL;
    }
    free(msg_serialized);  // Liberar a memória da mensagem serializada

    // - Esperar a resposta do servidor;
    uint8_t buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(socketfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        fprintf(stderr, "Erro ao receber a resposta do servidor.\n");
        return NULL;
    }
    //TODO o que seriam erros de comunicação a serem tratados?
    // - De-serializar a mensagem de resposta;   
    MessageT *response_msg = message_t__unpack(NULL, bytes_received, buffer);
   
    return response_msg== NULL?NULL: response_msg;

}

/* Fecha a ligação estabelecida por network_connect().
* Retorna 0 (OK) ou -1 (erro).
*/
int network_close(struct rtable_t *rtable){

    // Fecho bem-sucedido
    return close(rtable->sockfd);

}