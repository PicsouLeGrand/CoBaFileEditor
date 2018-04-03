#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_CLIENTS 10
#define MAX_PINGS 5

#define PORT_SERV_TCP 9001
#define PORT_SERV_UDP 9002

struct client {
	int id;
	int port; //port of user
	char *ip; //ip of user
	int nb_open_files; 
	int is_modifying;
	int line_nb; //if modifying, which line
	int unanswered_pings;
};

struct client clients[MAX_CLIENTS];
