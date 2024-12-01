#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "arpa/inet.h"
#include "zookeeper/zookeeper.h"
struct rtable_t{
 
  char *server_address;
  int server_port;
  int sockfd;
};

struct rtable_pair_t {
    struct rtable_t *head;
    struct rtable_t *tail;
};

#endif
