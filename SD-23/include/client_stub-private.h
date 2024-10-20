#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "arpa/inet.h"
struct rtable_t {
 
  int sockfd;
	struct sockaddr_in server;

};

#endif
