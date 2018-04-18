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

/*
 * Used to split the socket buffer and extract informations, then use them accordingly
 * to the protocol
 */
void deformatage(struct thread_args *args, char *buff){
	char *head;
	char *tail;
	char *original;

	original = malloc(strlen(buff) * sizeof(char) + 1);
	strcpy(original, buff);
	head = strtok(buff, " ");
	tail = strtok(NULL, "\n");

	if(strcmp(head, "Welcome") == 0){
		fprintf(stdout, "> %s", original);
	} else if(strcmp(head, "con!") == 0) {
		fprintf(stdout, "> You are now connected\n");
		pthread_cond_signal(&condition);
	} else if(strcmp(head, "err!") == 0) {
		fprintf(stdout, "> Server error : %s\n", tail);
		//temporaire, ie gérer les cas d'erreurs?
		quitter(args);
	} else if(strcmp(head, "qui!") == 0) { 
		fprintf(stdout, "\r> OK for disconnect!\n");
		quitter(args);
	} else {
		fprintf(stderr, "boi : %s\n", original);
	}
}

/*
 * read what the user wrote and then act accordingly
 */
void input_deformatage(struct thread_args *args, char *input){
	char *head;
	char *tail;
	char *original;

	strtok(input, "\n");
	strcat(input, "\0");

	original = malloc(strlen(input) * sizeof(char) + 1);
	strcpy(original, input);
	head = strtok(input, " ");
	tail = strtok(NULL, "\n");

	if(strcmp(original, "\n") != 0){
		if(strcmp(original, "quit") == 0 || strcmp(original, "exit") == 0) {
			send_msg(args, PROT_QUI);
		} else {
			fprintf(stdout, "> Command not recognized : %s\n", original);
		}
	}
}