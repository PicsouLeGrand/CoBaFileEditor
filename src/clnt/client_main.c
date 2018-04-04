#include "client_header.h"

int sock = 0;

/*
 * Send a message through the global TCP socket to the server
 */ 
void send_msg(char *msg){
	if(write(sock, msg, strlen(msg) * sizeof(char)) == -1){
		perror("write send_msg");
		exit(EXIT_FAILURE);
	}
	printf("Ping response sent!\n");
}

void *gestion_ping(){
	int sock;
	int rec;
	int ok;
	struct sockaddr_in address_sock;
	struct ip_mreq mreq;
	char buffer[BUFF_SIZE_PING];
	
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
		printf("%s\n", buffer);
		send_msg(PNG_RESPONSE);
	}

	pthread_exit(NULL);
}

int main(int argc, char** argv){
	int received;
	struct sockaddr_in *addressin;
	struct addrinfo *first_info;
	struct addrinfo hints;
	pthread_t threadUDP;

	if(argc != 2){
		fprintf(stderr, "usage : ./client_main address");
		exit(EXIT_FAILURE);
	}

	pthread_create(&threadUDP, NULL, gestion_ping, NULL);
	sock = socket(PF_INET, SOCK_STREAM, 0);

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

	if(connect(sock, (struct sockaddr *) addressin, (socklen_t) sizeof(struct sockaddr_in)) != 0) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	char buff[100];
	while(1){
		received = read(sock, buff, 99*sizeof(char));
		buff[received] = '\0';
		if(strcmp(buff, "\n") != 0 || strcmp(buff, "") != 0){
			printf("message received : %s\n", buff);
		
		}
	}

	//char *mess = "Coucou !\n";
	//write(sock, mess, strlen(mess) * sizeof(char));
	
	close(sock);

	return 0;
}
