#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_SERV_TCP 9001
#define PORT_CLNT_TCP "9001"
#define PORT_SERV_UDP 9002
#define ADDRESS "localhost"
