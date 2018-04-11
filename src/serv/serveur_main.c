#include "serveur_header.h"

/*
 * TODO
 *
 * Ejecter les clients qui sont afk, ça implique compter à chaque ping non répondu et les virer de la table des clients
 * (si je dis pas de conneries)
 *
 * TODO
 */


int ID_COUNTER = 0;

/*
 * Executed by a thread, one instance per client connected
 */
void *client_mainloop(void *t_args) {
	struct thread_args *args = (struct thread_args *) t_args;
	int received;
	
	char *welcome = "Welcome to the CoBa File Editor.\nType help for help.\n";
	if(send(args->sock2, welcome, strlen(welcome) * sizeof(char), 0) < 0) {
		perror("send error");
		exit(EXIT_FAILURE);
	}

	while(1){
		char buff[100];
		received = recv(args->sock2, buff, 99*sizeof(char), 0);
		buff[received] = '\0';
		if(strcmp(buff, "\0") != 0 || strcmp(buff, "\n") != 0){
			if(strcmp(buff, "png!") == 0){
				printf("Ping response from : %s and port : %d\n", inet_ntoa(args->caller.sin_addr),
				args->caller.sin_port);
			} else {
				printf("%s from : %s and port : %d\n", buff, inet_ntoa(args->caller.sin_addr), args->caller.sin_port);
			}

		}
		else {
			printf("chaine : %s", buff);
		}
	}

	pthread_exit(NULL);
}

/*
 * Send a ping via a UDP port to all clients listening
 */
void *pingUDP(){
	int sock; //UDP connection socket
	struct addrinfo *first_info;
	struct sockaddr *saddr;
	char *png = "png?";

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
		sleep(PING_INTERVAL);
	}

	pthread_exit(NULL);
}

/*
 * Add a client to the master list
 */
void add_client(struct client new_client){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(clients[i].port == 0){
			clients[i] = new_client;
			break;
		}
	}
	//TODO CLEAN OLD CLIENTS
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
		if(clients[i].port == 0)
			break;
		else {
			print_client(clients[i]);
			printf("---------------------------------------\n");
		
		}
	}
}

/*
 * Initialize a new client with args provided in the caller struct
 */
struct client create_client(struct sockaddr_in caller){
	struct client new_client;

	new_client.id = ID_COUNTER;
	ID_COUNTER++;
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
	
	while(1){
		if((sock2 = accept(sock, (struct sockaddr *) &caller, &size)) == -1){
			perror("accept error");
			exit(EXIT_FAILURE);
		}

		struct client new_client = create_client(caller);
		//add_client(new_client);
		//print_all_clients();
		
		//TODO CHECK IF MAX_CLIENTS ARE CONNECTED

		t_args = malloc(sizeof(struct thread_args));
		memset(t_args, 0, sizeof(struct thread_args));
		t_args->sock2 = sock2;
		t_args->caller = caller;

		if(pthread_create(&threads_clients[new_client.id], NULL, client_mainloop, (void *) t_args) != 0){
			perror("pthread error 2");
			exit(EXIT_FAILURE);
		}
		
		
	}
	close(sock2);

	return 0;
}
