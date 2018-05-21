#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ncurses.h>

#define PORT_CLNT_TCP "9001"
#define PORT_CLNT_UDP "9002"
#define PORT_CLNT_UDP_INT 9002
#define ADDR_MULTICAST "225.1.2.4"

#define BUFF_SIZE_PING 100
#define BUFF_SIZE_INPUT 1024
#define BUFF_SIZE_RECV 1024
#define BUFF_SIZE_SMALL 64
#define BUFF_SIZE_MEDIUM 256

#define PROT_PNG_R "png!"
#define PROT_CON "con?"
#define PROT_CON_R "con!"
#define PROT_QUI "qui?"
#define PROT_QUI_R "qui!"
#define PROT_LST "lst?"
#define PROT_LST_R "lst!"
#define PROT_ERR "err!"
#define PROT_CRE "cre?"
#define PROT_CRE_R "cre!"
#define PROT_DEL "del?"
#define PROT_DEL_R "del!"
#define PROT_LFI "lfi?"
#define PROT_LFI_R "lfi!"
#define PROT_MOD "mod?"
#define PROT_MOD_R "mod!"

#define CMD_HELP "help"
#define CMD_LSTU "listu"
#define CMD_QUIT "quit"
#define CMD_EXIT "exit"
#define CMD_LSTF "listf"
#define CMD_CREA "create"
#define CMD_MODI "modify"
#define CMD_DELE "delete"

#define CMD_HELP_SHORT "h"
#define CMD_LSTU_SHORT "lu"
#define CMD_LSTF_SHORT "lf"
#define CMD_CREA_SHORT "c"
#define CMD_MODI_SHORT "m"
#define CMD_DELE_SHORT "d"

#define CURSES_CMD_DEL "d"
#define CURSES_CMD_INS "i"

#define CURSES_DEL "dlg?"
#define CURSES_DEL_R "dlg!"
#define CURSES_INS "ilg?"
#define CURSES_INS_R "ilg!"	

#define SPECIAL_SEPARATOR "\t" //special character used to split messages when several are received at the same time
#define SPECIAL_EOF "~#{[|``|[{#~" //special char used for synchronization in ncurses mode

#define ERR_MSG_1 "maximum number of clients reached. Try again later."
#define ERR_MSG_2 "error while creating the file, perhaps it exists already?"
#define ERR_MSG_3 "> Error, you need to specify a filename\n"
#define ERR_MSG_4 "> Error, you need to specify a line number"

struct thread_args {
	int sock;
};

extern pthread_mutex_t mutex;
extern pthread_cond_t condition;
extern SCREEN *s;

void send_msg(struct thread_args *args, char *msg);
void *gestion_ping();
void *gestion_recv(void *t_args);

void deformatage(struct thread_args *args, char *buff);
void input_deformatage(struct thread_args *args, char *input);
void curses_deformatage(struct thread_args *args, char *input);

void quitter(struct thread_args *args);
void print_help();
void create_file(struct thread_args *args, char *name);
void modify_file(struct thread_args *args, char *name);
void delete_file(struct thread_args *args, char *name);