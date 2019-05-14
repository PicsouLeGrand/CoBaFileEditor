#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <dirent.h>

#define MAX_CLIENTS 3
#define MAX_PINGS 3
#define PING_INTERVAL 2

#define PORT_SERV_TCP 9001
#define PORT_SERV_UDP "9002"
#define ADDR_MULTICAST "225.1.2.4"

#define BUFF_SIZE_RECV 1024
#define BUFF_SIZE_ERR 1024
#define BUFF_SIZE_SMALL 64
#define BUFF_SIZE_MEDIUM 256

#define PROT_PNG "png?"
#define PROT_PNG_R "png!"
#define PROT_CON "con?"
#define PROT_CON_R "con!"
#define PROT_ERR "err!"
#define PROT_QUI "qui?"
#define PROT_QUI_R "qui!"
#define PROT_LST "lst?"
#define PROT_LST_R "lst!"
#define PROT_CRE "cre?"
#define PROT_CRE_R "cre!"
#define PROT_DEL "del?"
#define PROT_DEL_R "del!"
#define PROT_LFI "lfi?"
#define PROT_LFI_R "lfi!"
#define PROT_MOD "mod?"
#define PROT_MOD_R "mod!"

#define CURSES_DEL "dlg?"
#define CURSES_DEL_R "dlg!"
#define CURSES_INS "ilg?"
#define CURSES_INS_R "ilg!"
#define CURSES_MOD "mlg?"
#define CURSES_MOD_R "mlg!"

#define ERR_MSG_1 "maximum number of clients reached. Try again later."
#define ERR_MSG_2 "error while creating the file, perhaps it exists already?"
#define ERR_MSG_3 "error while deleting the file, perhaps it's already deleted."
#define ERR_MSG_4 "error while opening the file, perhaps it doesn't exists?"

#define SPECIAL_SEPARATOR "\t" //special character used to split messages when several are received at the same time
#define SPECIAL_EOF "~#{[|``|[{#~" //special char used for synchronization in ncurses mode

struct client {
	//int sock;
	int id;
	int port; //port of user
	char *ip; //ip of user
	int nb_open_files;
	char *file; //current file
	char *height; //height of terminal
	int is_modifying;
	int line_nb; //if modifying, which line
	int unanswered_pings; //usually worth 1 because of implementation
};

struct thread_args {
	int sock2;
	struct sockaddr_in caller;
	struct client c;
};

extern int fd;
extern int NB_CLIENTS;
extern pthread_mutex_t mutex;
struct client clients[MAX_CLIENTS];
struct thread_args *global_args[MAX_CLIENTS];

void *pingUDP();
void add_client(struct client new_client, struct thread_args *args);
void remove_client(struct client old_client);
void print_client(struct client c);
void print_all_clients();
struct client create_empty_client();
struct client create_client(struct sockaddr_in caller);
void *client_mainloop(void *t_args);

int send_msg(struct thread_args *args, char *msg);
int send_welco(struct thread_args *args);
int send_err(int sock, char *message);

void write_to_log(struct client c, char *msg);
void deformatage(struct thread_args *args);
void modification(struct thread_args *args, char *tail, char *after);
void liste_users(struct thread_args *args, char *buff);
void liste_files(struct thread_args *args, char *buff);
void curses_line_delete(struct thread_args *args, char *buff, char *tail);
void curses_line_insert(struct thread_args *args, char *buff, char *tail);
void curses_line_modification(struct thread_args *args, char *buff, char *tail, char *after);
void send_modifs_to_all(struct thread_args *args);
