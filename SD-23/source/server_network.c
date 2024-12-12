
/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "table.h"
#include "htmessages.pb-c.h"
#include "message-private.h"
#include "server_skeleton.h"
#include "server_network.h"
#include "stats-private.h"

// Variavel global para a tabela
static struct table_t *global_table;

/* Função de atendimento para cada cliente */
void *client_handler(void *client_socket)
{
    int connection_socket = *((int *)client_socket);
  
    printf("Cliente conectado com sucesso.\n");
    fflush(stdout);

    // Incrementar o contador de clientes
    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.connected_clients++;
    pthread_mutex_unlock(&server_stats.stats_mutex);

    // Loop de atendimento ao cliente
    MessageT *request_msg;
    while ((request_msg = network_receive(connection_socket)) != NULL)
    {

        if (invoke(request_msg, global_table) < 0)
        {
            printf("Erro ao processar a mensagem.\n");
            fflush(stdout);
            network_send(connection_socket, request_msg);
            continue;
        }

        // Envia a resposta para o cliente
        if (network_send(connection_socket, request_msg) < 0)
        {
            printf("Erro ao enviar a resposta para o cliente.\n");
            fflush(stdout);
            break;
        }

        // Limpa a mensagem recebida
        if (request_msg != NULL)
        {
            message_t__free_unpacked(request_msg, NULL);
        }
    }

    // Diminuir o contador de clientes
    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.connected_clients--;
    pthread_mutex_unlock(&server_stats.stats_mutex);

    // Fechar a conexão ao final do atendimento
    close(connection_socket);
    printf("Conexão com cliente encerrada.\n");
    fflush(stdout);
    pthread_exit(NULL); // Termina a thread
}

int server_network_init(short port)
{
    int sockfd;
    struct sockaddr_in server;

    printf("PASSA1\n");
    fflush(stdout);

    // Criar socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Erro ao criar o socket");
        return -1;
    }

    // Configurar a opção SO_REUSEADDR para reutilizar a porta
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        printf("Erro ao configurar SO_REUSEADDR\n");
        close(sockfd);
        return -1;
    }

    // Preencher a estrutura server para o bind
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Fazer o bind do socket ao endereço e à porta
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Erro ao fazer o bind\n");
        close(sockfd); // Fechar o socket em caso de erro
        return -1;
    }

    // Colocar o socket em modo listen para aceitar conexões
    if (listen(sockfd, 5) < 0)
    {
        printf("Erro ao executar o listen");
        close(sockfd); // Fechar o socket em caso de erro
        return -1;
    }

    return sockfd;
}


/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 * reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket)
{

    // Ler o tamanho da mensagem
    uint32_t msg_size_network;
    if (read_all(client_socket, &msg_size_network, sizeof(msg_size_network)) < 0)
    {
        fprintf(stderr, "Erro ao ler o tamanho da mensagem.\n");
        return NULL;
    }

    size_t msg_size = ntohl(msg_size_network); // Converter size

    // Alocar um buffer com o tamanho da mensagem
    uint8_t *msg_buffer = malloc(msg_size);
    if (msg_buffer == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para a mensagem.\n");
        return NULL;
    }

    // Ler a mensagem completa usando o tamanho previamente lido
    if (read_all(client_socket, msg_buffer, msg_size) < 0)
    {
        fprintf(stderr, "Erro ao ler a mensagem completa.\n");
        free(msg_buffer);
        return NULL;
    }

    // Deserializar a mensagem
    MessageT *message = message_t__unpack(NULL, msg_size, msg_buffer);
    free(msg_buffer); // Libertar o buffer após a deserialização
    if (message == NULL)
    {
        printf("Erro ao de-serializar a mensagem.\n");
        return NULL;
    }

    return message;
}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg)
{

    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *msg_serialized = malloc(msg_size);
    if (msg_serialized == NULL)
    {
        printf("Erro ao alocar memória para a resposta.\n");
        return -1;
    }

    message_t__pack(msg, msg_serialized);

    uint32_t msg_size_network = htonl(msg_size);
    if (write_all(client_socket, &msg_size_network, sizeof(msg_size_network)) < 0)
    {
        printf("Erro ao enviar o tamanho da mensagem.\n");
        free(msg_serialized);
        return -1;
    }

    if (write_all(client_socket, msg_serialized, msg_size) < 0)
    {
        printf("Erro ao enviar a mensagem ao cliente.\n");
        free(msg_serialized);
        return -1;
    }

    free(msg_serialized);
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
int network_main_loop(int listening_socket, struct table_t *table)
{
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    global_table = table;
    int client_socket;
    printf("Servidor inicializado com sucesso, para desligar servidor: ^C\n");
    printf("Servidor à espera de ligações\n");
    fflush(stdout);

    while (1)
    {
        client_socket = accept(listening_socket, (struct sockaddr *)&client, &client_len);
        if (client_socket < 0)
        {
            perror("Erro ao aceitar conexão");
            
            continue;
        }

        // Cria uma nova thread para atender o cliente
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, &client_socket) != 0)
        {
            perror("Erro ao criar thread do cliente");
            close(client_socket);
            continue;
        }

        pthread_detach(client_thread); // Deixa a thread se limpar automaticamente após terminar
    }

    return -1;
}

/* Liberta os recursos alocados por server_network_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int server_network_close(int socket)
{
    server_skeleton_destroy(global_table);
    printf("\nDesconectando...\n");
    if (socket < 0 || close(socket) < 0)
        return 1;
    return 0;
}