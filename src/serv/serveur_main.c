#include "serveur_header.h"

/*
 * TODO
 *
 * Ejecter les clients qui sont afk, ça implique compter à chaque ping non répondu et les virer de la table des clients
 * (si je dis pas de conneries)
 *
 * Créer un fichier de formatage / déformatage qui s'occupe de gérer les messages échangés grâce au protocole
 * 1 fichier pour le serveur 1 pour le client à priori
 * TODO
 */


int ID_COUNTER = 0;
int NB_CLIENTS = 0;
static struct client empty_client;

/*
 * Executed by a thread, one instance per client connected
 */
void *client_mainloop(void *t_args) {
	struct thread_args *args = (struct thread_args *) t_args;
	int received;
	int is_connected = 0;
	//char *message;
	//char *err;
	
	send_welco(args);

	add_client(args->c);
	NB_CLIENTS++;
	//print_all_clients();
	
	while(1){
		char buff[BUFF_SIZE_RECV];
		received = recv(args->sock2, buff, 99*sizeof(char), 0);
		buff[received] = '\0';

		if(strcmp(buff, "") != 0){
			//first of all, client need to send con? request
			if(is_connected == 0){
				if(strcmp(buff, PROT_CON) == 0){
					//TODO verifs pour que le client se connecte bien
					//send confirmation message for connection
					send_con_r(args);
					is_connected = 1;
				}/* else {
					printf("strange : %s", buff);
					message = malloc(1024*sizeof(char));
					err = "Something went wrong when connecting...";
					strcat(message, PROT_ERR);
					strcat(message, err);
					send(args->sock2, message, strlen(message) * sizeof(char), 0);
					close(args->sock2);
					printf("> Client disconnected, was : %s and port %d\n",
					inet_ntoa(args->caller.sin_addr), args->caller.sin_port);
					break;
					//TODO clean correctement le client
				}*/

			} else {
			}

			if(strcmp(buff, PROT_PNG_R) == 0){
				printf("> Ping response from : %s and port : %d\n", inet_ntoa(args->caller.sin_addr),
				args->caller.sin_port);

				clients[args->c.id].unanswered_pings = 0;
			} else {
				printf("> %s from : %s and port : %d\n", buff, inet_ntoa(args->caller.sin_addr), args->caller.sin_port);
			}

		}
	}

	pthread_exit(NULL);
}

/*
 * Send a ping via a UDP port to all clients listening
 */
void *pingUDP(){
	int sock; //UDP connection socket
	int i;
	struct addrinfo *first_info;
	struct sockaddr *saddr;
	char *png = PROT_PNG;

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	//memset(&hints, 0, sizeof(struct addrinfo));
	//hints.ai_family = AF_INET;
	//hints.ai_socktype = SOCK_DGRAM;
	
	if(getaddrinfo(ADDR_MULTICAST, PORT_SERV_UDP, NULL, &first_info) != 0){
		perror("getaddrinfo UDP error");
		exit(EXIT_FAILURE);
	}

	if(first_info == NULL){
		perror("first_info NULL error");
		exit(EXIT_FAILURE);
	}

	saddr = first_info->ai_addr;
	while(1){
		sendto(sock, png, strlen(png), 0, saddr, (socklen_t) sizeof(struct sockaddr_in));

		print_all_clients();
		sleep(PING_INTERVAL);
		for(i = 0; i < NB_CLIENTS; i++){
			clients[i].unanswered_pings++;
			if(clients[i].unanswered_pings >= MAX_PINGS && clients[i].id != -1){
				printf("> Client n°%d is removed because of possible disconnect.\n", clients[i].id);
				remove_client(clients[i]);
			}
		}

	}

	pthread_exit(NULL);
}

/*
 * Add a client to the master list
 */
void add_client(struct client new_client) {
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(clients[i].port < 0){
			clients[i] = new_client;
			break;
		}
	}
	//TODO CLEAN OLD CLIENTS
}

/*
 * Remove a client from the master list
 */
void remove_client(struct client old_client) {
	clients[old_client.id] = empty_client;
}

/*
 * Print informations about a specific client
 */
void print_client(struct client c) {
	printf("ID            : %d\n", c.id);
	printf("Port          : %d\n", c.port);
	printf("IP            : %s\n", c.ip);
	printf("Nb open files : %d\n", c.nb_open_files);
	switch(c.is_modifying){
		case(0):
			printf("Is modifying  : NO\n");
			break;
		case(1):
			printf("Is modifying  : YES\n");
			printf("Line nb       : %d\n", c.line_nb);
			break;
	}
	printf("Missed pings  : %d\n", c.unanswered_pings);
}

/*
 * Print info of all clients currently on server
 */
void print_all_clients(){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(clients[i].port < 0)
			break;
		else {
			print_client(clients[i]);
			printf("---------------------------------------\n");
		
		}
	}
}

/*
 * Create an empty client
 */
struct client create_empty_client(){
	struct client c;

	c.id = -1;
	c.port = -1;
	c.ip = "-1";
	c.nb_open_files = -1;
	c.is_modifying = -1;
	c.line_nb = -1;
	c.unanswered_pings = -1;

	return c;
}

/*
 * Initialize a new client with args provided in the caller struct
 */
struct client create_client(struct sockaddr_in caller){
	struct client new_client;

	new_client.id = ID_COUNTER;
	new_client.port = caller.sin_port;
	new_client.ip = malloc(INET_ADDRSTRLEN * sizeof(char));
	new_client.ip = inet_ntoa(caller.sin_addr);
	new_client.nb_open_files = 0;
	new_client.is_modifying = 0;
	new_client.line_nb = 0;
	new_client.unanswered_pings = 0;

	return new_client;
}

int main(){
	int sock; //TCP connection socket
	int sock2;
	int i;
	socklen_t size;
	pthread_t thread_UDP;
	pthread_t threads_clients[MAX_CLIENTS];
	
	struct sockaddr_in address_sock;
	struct sockaddr_in caller;
	struct thread_args *t_args;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	address_sock.sin_family = AF_INET;
	address_sock.sin_port = htons(PORT_SERV_TCP);
	address_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	
	size = sizeof(caller);

	if(bind(sock, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in)) != 0) {
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if(listen(sock, 0) != 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&thread_UDP, NULL, pingUDP, NULL) != 0){
		perror("pthread error 1");
		exit(EXIT_FAILURE);
	}
	
	empty_client = create_empty_client();
	for(i = 0; i < MAX_CLIENTS; i++)
		clients[i] = empty_client;

	while(1){
		if((sock2 = accept(sock, (struct sockaddr *) &caller, &size)) == -1){
			perror("accept error");
			exit(EXIT_FAILURE);
		}
		
		//ID_COUNTER = first empty client slot
		for(i = 0; i < MAX_CLIENTS; i++){
			if(clients[i].id == -1){
				ID_COUNTER = i;
				break;
			}
		}
		struct client new_client = create_client(caller);
		//add_client(new_client);
		//print_all_clients();
		
		//TODO CHECK IF MAX_CLIENTS ARE CONNECTED

		t_args = malloc(sizeof(struct thread_args));
		memset(t_args, 0, sizeof(struct thread_args));
		t_args->sock2 = sock2;
		t_args->caller = caller;
		t_args->c = new_client;

		printf("> A new client wants to connect, launching thread.\n");

		//launch a thread for the new client
		if(pthread_create(&threads_clients[new_client.id], NULL, client_mainloop, (void *) t_args) != 0){
			perror("pthread error 2");
			exit(EXIT_FAILURE);
		}
		
		
	}
	close(sock2);

	return 0;
}
