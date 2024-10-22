
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "htmessages.pb-c.h"

/* Função para preparar um socket de receção de pedidos de ligação
* num determinado porto.
* Retorna o descritor do socket ou -1 em caso de erro.
*/
int server_network_init(short port){
    int sockfd;
    struct sockaddr_in server;

    // Cria o socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar o socket");
        return -1;
    }

    // Preenche a estrutura server para o bind
    memset(&server, 0, sizeof(server)); 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(port); 

    // Faz o bind do socket ao endereço e à porta
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao fazer o bind");
        close(sockfd); // Fecha o socket em caso de erro
        return -1;
    }

    // Coloca o socket em modo de escuta para aceitar conexões
    if (listen(sockfd, 5) < 0) {
        perror("Erro ao executar o listen");
        close(sockfd); // Fecha o socket em caso de erro
        return -1;
    }

    // Retorna o descritor do socket pronto para aceitar conexões
    return sockfd;
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
int network_main_loop(int listening_socket, struct table_t *table);

/* A função network_receive() deve:
* - Ler os bytes da rede, a partir do client_socket indicado;
* - De-serializar estes bytes e construir a mensagem com o pedido,
* reservando a memória necessária para a estrutura MessageT.
* Retorna a mensagem com o pedido ou NULL em caso de erro.
*/
MessageT *network_receive(int client_socket);

/* A função network_send() deve:
* - Serializar a mensagem de resposta contida em msg;
* - Enviar a mensagem serializada, através do client_socket.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int network_send(int client_socket, MessageT *msg);

/* Liberta os recursos alocados por server_network_init(), nomeadamente
* fechando o socket passado como argumento.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int server_network_close(int socket);

