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


/**
* Data Watcher function for /MyData node
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	int zoo_data_len = ZDATALEN;
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
	 	   /* Get the updated children and reset the watch */ 
 			if (ZOK != zoo_wget_children(wzh, "/chain", child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", "/chain");
 			}
		 } 
	 }
	 free(children_list);
}



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


    //criar tabela com n_listas pedidas
    int n_listas = atoi(argv[3]);
    struct table_t *table =server_skeleton_init(n_listas);

    if(table==NULL){
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

    /*


    ZOOKEEPER COMEÇA AQUI


    */

    
    struct zhandle_t* zh =zookeeper_init(zoo_server_and_port, my_watcher_func,2000, 0, NULL, 0);

    if (zh == NULL)	{
        fprintf(stderr, "Erro ao conectar ao servidor Zookeeper!\n");
        exit(EXIT_FAILURE);
     }
   
    sleep(1);

    if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL)) {
                fprintf(stderr, "/chain não existe! \
                    A criar nó /chain \n");

        create_node_chain(zh);
    }

    int new_path_len = 1024;
    char* new_path = malloc (new_path_len); 
    zoo_create(zh, "/chain/node", NULL, 0, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len);
    //temos que fazer com que o servidor guarde o new_path


    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    if (ZOK != zoo_wget_children(zh, "/chain", child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", "/chain");
			}

    
    char **array = malloc(2 * sizeof(char *));
    //obter nome do nó para encontrar o antecessor e posterior
    char* node_name=extract_node_name(new_path);
    get_nodes_before_after(children_list, array, node_name);
    free (new_path);

    //guardar os valores de array[0] e array[1]  


    /*


    ZOOKEEPER ACABOU AQUI


    */


    // Entrar no loop principal para processar as conexões
    if (network_main_loop(listening_socket, table) < 0) {
        printf("Erro no loop de rede");
        close(listening_socket);
        return -1;
    }

    // Fechar o socket ao sair (em caso de erro)
    server_network_close(listening_socket);

    return 0;
}