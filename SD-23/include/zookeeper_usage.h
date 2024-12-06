#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

typedef struct String_vector zoo_string; 

struct server_t {
    struct table_t *table;           
    zhandle_t *zh;                   
    char *znode_path;                
    char *next_server_name;          
    struct rtable_t *next_server;    
};


/**
* Watcher function for connection state change events
*/
void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx);

void create_node_chain(zhandle_t *zzh);

char *extract_node_name(char *path);

char** get_nodes_before_after(struct String_vector *strings, char** array, char* compare);

struct server_t *server_init(struct server_t *servidor, int n_lists, char *zoo_server,char *server_ip_port);

void server_destroy(struct server_t *server);


