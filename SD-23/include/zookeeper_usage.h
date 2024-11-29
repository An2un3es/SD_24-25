#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

typedef struct String_vector zoo_string; 


/**
* Watcher function for connection state change events
*/
void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx);

void create_node_chain(zhandle_t *zzh);

char *extract_node_name(char *path);

void get_nodes_before_after(struct String_vector *strings, char** array, char* compare);


