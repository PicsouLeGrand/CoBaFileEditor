#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_CLNT_TCP "9001"
#define PORT_CLNT_UDP "9002"
#define PORT_CLNT_UDP_INT 9002
#define ADDR_MULTICAST "225.1.2.4"

#define BUFF_SIZE_PING 100

#define PNG_RESPONSE "png!"

void send_msg(char *msg);
void *gestion_ping();
