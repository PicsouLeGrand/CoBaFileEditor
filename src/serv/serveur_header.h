#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_CLIENTS 2
#define MAX_PINGS 5
#define PING_INTERVAL 3

#define PORT_SERV_TCP 9001
#define PORT_SERV_UDP "9002"
#define ADDR_MULTICAST "225.1.2.4"

#define BUFF_SIZE_RECV 1024
#define BUFF_SIZE_ERR 1024

#define PROT_PNG "png?"
#define PROT_PNG_R "png!"
#define PROT_CON "con?"
#define PROT_CON_R "con!"
#define PROT_ERR "err! "

#define ERR_MSG_1 "maximum number of clients reached. Try again later.\n"

struct client {
	int id;
	int port; //port of user
	char *ip; //ip of user
	int nb_open_files; 
	int is_modifying;
	int line_nb; //if modifying, which line
	int unanswered_pings; //usually worth 1 because of implementation
};

struct thread_args {
	int sock2;
	struct sockaddr_in caller;
	struct client c;
};

struct client clients[MAX_CLIENTS];

void *pingUDP();
void add_client(struct client new_client);
void remove_client(struct client old_client);
void print_client(struct client c);
void print_all_clients();
struct client create_empty_client();
struct client create_client(struct sockaddr_in caller);
void *client_mainloop(void *t_args);

int send_con_r(struct thread_args *args);
int send_welco(struct thread_args *args);
int send_err(int sock, char *message);
