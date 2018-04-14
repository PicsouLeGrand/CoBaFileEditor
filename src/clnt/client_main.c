#include "client_header.h"

int sock_global = 0;

/*
 * Executed by a thread, manage reception of messages from the server on TCP
 */
void *gestion_recv(void *t_args){
	struct thread_args *args = (struct thread_args *) t_args;
	int received;
	char buff[BUFF_SIZE_RECV];

	while(1){
		received = read(args->sock, buff, 99*sizeof(char));
		buff[received] = '\0';
		if(strcmp(buff, "") != 0){
			//send the buffer to deformatage() for reading
			deformatage(buff);
			//printf("%s\n", buff);
			//printf("%lu\n", strlen(buff));
		}
	}
}

/*
 * Executed by a thread, manage the UDP port for ping reception and
 * answer it (on TCP)
 */
void *gestion_ping(){
	int sock;
	int rec;
	int ok;
	struct sockaddr_in address_sock;
	struct ip_mreq mreq;
	char buffer[BUFF_SIZE_PING];
	struct thread_args *args;

	args = malloc(sizeof(struct thread_args *));
	args->sock = sock_global;
	
	ok = 1;
	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &ok, sizeof(ok)) != 0) {
		perror("setsockpt error 1");
		exit(EXIT_FAILURE);
	}

	address_sock.sin_family = AF_INET;
	address_sock.sin_port = htons(PORT_CLNT_UDP_INT);
	address_sock.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in)) != 0){
		perror("bind UDP error");
		exit(EXIT_FAILURE);
	}

	mreq.imr_multiaddr.s_addr = inet_addr(ADDR_MULTICAST);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
	if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
		perror("setsockopt error 2");
		exit(EXIT_FAILURE);
	}

	while(1){
		rec = recv(sock, buffer, BUFF_SIZE_PING, 0);
		buffer[rec] = '\0';
		//printf("%s\n", buffer);
		send_msg(args, PROT_PNG_R);
		//printf("Ping response sent\n");
	}

	pthread_exit(NULL);
}

int main(int argc, char** argv){
	struct sockaddr_in *addressin;
	struct addrinfo *first_info;
	struct addrinfo hints;
	struct thread_args *t_args;
	pthread_t threadUDP;
	pthread_t threadRecv;
	char *input;

	if(argc != 2){
		fprintf(stderr, "usage : ./client_main address");
		exit(EXIT_FAILURE);
	}

	pthread_create(&threadUDP, NULL, gestion_ping, NULL);

	input = malloc(BUFF_SIZE_INPUT * sizeof(char));

	sock_global = socket(PF_INET, SOCK_STREAM, 0);

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(argv[1], PORT_CLNT_TCP, &hints, &first_info) != 0) {
		perror("getaddrinfo error");
		exit(EXIT_FAILURE);
	}

	if(first_info == NULL){
		perror("couldn't get info error");
		exit(EXIT_FAILURE);
	}

	addressin = (struct sockaddr_in *) first_info->ai_addr;

	if(connect(sock_global, (struct sockaddr *) addressin, (socklen_t) sizeof(struct sockaddr_in)) != 0) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	t_args = malloc(sizeof(struct thread_args));
	memset(t_args, 0, sizeof(struct thread_args));
	t_args->sock = sock_global;

	pthread_create(&threadRecv, NULL, gestion_recv, (void *) t_args);

	sleep(1);
	//send a connect request to the server
	send_con(t_args);
	
	//for lisibility, else the first input message is not properly displayed
	sleep(1);
	while(1) {
		//TODO envoi de commandes au serveur
		memset(input, 0, BUFF_SIZE_INPUT * sizeof(char));
		printf("Enter your command :");
		fgets(input, BUFF_SIZE_INPUT, stdin);
		//TODO gérer l'input, le formater et l'envoyer
	}
	
	//char *mess = "Coucou !\n";
	//write(sock, mess, strlen(mess) * sizeof(char));
	
	close(sock_global);

	return 0;
}
