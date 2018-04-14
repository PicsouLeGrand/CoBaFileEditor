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
#define BUFF_SIZE_INPUT 1024
#define BUFF_SIZE_RECV 1024

#define PROT_PNG_R "png!"
#define PROT_CON "con?"

struct thread_args {
	int sock;
};

void send_msg(struct thread_args *args, char *msg);
void *gestion_ping();
void *gestion_recv(void *t_args);

int send_con(struct thread_args *args);
void deformatage(char *buff);