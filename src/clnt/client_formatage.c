/*
 * THIS FILE CONTAINS METHODS TO USE THE PROTOCOL EFFICIENTLY
 * Some of them create messages and some trunc messages for example
 */

#include "client_header.h"

/*
 * Send a message through the global TCP socket to the server
 */ 
void send_msg(struct thread_args *args, char *msg){
	strtok(msg, "\n");
	strcat(msg, "\0");

	if(write(args->sock, msg, strlen(msg) * sizeof(char)) == -1){
		perror("write send_msg");
		exit(EXIT_FAILURE);
	}
}

int send_con(struct thread_args *args){
	return write(args->sock, PROT_CON, strlen(PROT_CON) * sizeof(char));
}

void deformatage(char *buff){
	char *head;
	char *original;

	original = malloc(strlen(buff) * sizeof(char) + 1);
	strcpy(original, buff);
	head = strtok(buff, " ");
	

	if(strcmp(head, "Welcome") == 0){
		printf("> %s", original);
	} else if(strcmp(head, "con!") == 0) {
		printf("> You are now connected\n");
	} else {
		printf("boi : %s\n", original);
	}
}