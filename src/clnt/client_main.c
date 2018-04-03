#include "client_header.h"

void *gestion_ping(){

}

int main(int argc, char** argv){
	int sock = 0;
	int received;
	struct sockaddr_in *addressin;
	struct addrinfo *first_info;
	struct addrinfo hints;
	pthread_t threadUDP;

	if(argc != 2){
		fprintf(stderr, "usage : ./client_main address");
		exit(EXIT_FAILURE);
	}

	pthread_create(threadUDP, NULL, gestion_ping, NULL);
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
	received = read(sock, buff, 99*sizeof(char));
	buff[received] = '\0';
	printf("message received : %s\n", buff);

	char *mess = "Coucou !\n";
	write(sock, mess, strlen(mess) * sizeof(char));
	
	close(sock);

	return 0;
}
