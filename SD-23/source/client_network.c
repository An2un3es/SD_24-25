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

/* Esta função deve:
* - Obter o endereço do servidor (struct sockaddr_in) com base na
* informação guardada na estrutura rtable;
* - Estabelecer a ligação com o servidor;
* - Guardar toda a informação necessária (e.g., descritor do socket)
* na estrutura rtable;
* - Retornar 0 (OK) ou -1 (erro).
*/
int network_connect(struct rtable_t *rtable){

    struct sockaddr_in server_addr;
    struct hostent *server;

    // Criar o socket TCP
    if ((rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar o socket");
        return -1;
    }

    if ((server = gethostbyname(rtable->server_address)) == NULL) {
        perror("Erro ao resolver endereço do servidor");
        close(rtable->sockfd);
        return -1;
    }

    // Preencher a estrutura sockaddr_in
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(rtable->server_port);

    // Estabelecer ligação ao servidor
    if (connect(rtable->sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(rtable->sockfd);
        return -1;
    }

    // Ligação bem-sucedida
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
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg){

    


}

/* Fecha a ligação estabelecida por network_connect().
* Retorna 0 (OK) ou -1 (erro).
*/
int network_close(struct rtable_t *rtable){

    // Verificar se o descritor do socket é válido
    if (rtable->sockfd < 0) {
        fprintf(stderr, "Erro: Socket inválido.\n");
        return -1;
    }

    // Fechar o socket
    if (close(rtable->sockfd) < 0) {
        perror("Erro ao fechar o socket");
        return -1;
    }

    // Resetar o descritor do socket na rtable
    rtable->sockfd = -1;

    // Fecho bem-sucedido
    return 0;

}