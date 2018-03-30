#include "../header.h"

int main(){
	int sock; //TCP connection socket
	int sock2;
	int received;
	socklen_t size;

	struct sockaddr_in address_sock;
	struct sockaddr_in caller;

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

	while(1){
		if((sock2 = accept(sock, (struct sockaddr *) &caller, &size)) == -1){
			perror("accept error");
			exit(EXIT_FAILURE);
		}

		char *test = "Hello ?\n";
		if(send(sock2, test, strlen(test) * sizeof(char), 0) < 0) {
			perror("send error");
			exit(EXIT_FAILURE);
		}

		char buff[100];
		received = recv(sock2, buff, 99*sizeof(char), 0);
		buff[received] = '\0';
		printf("Received message : %s\n", buff);
	}
	close(sock2);

	return 0;
}
