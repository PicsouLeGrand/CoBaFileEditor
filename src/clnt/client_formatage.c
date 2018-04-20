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
 * Used to split the incoming socket buffer and extract informations,
 * then use them accordingly to the protocol
 */
void deformatage(struct thread_args *args, char *buff){
	char *head;
	char *tail;
	char *after;
	char *original;

	original = malloc(strlen(buff) * sizeof(char) + 1);
	strcpy(original, buff);
	head = strtok(buff, " ");
	tail = strtok(NULL, SPECIAL_SEPARATOR);
	
	// printf("o %s\n", original);
	// if(tail != NULL) printf("t %s\n", tail);

	if(strcmp(head, "Welcome") == 0){
		fprintf(stdout, "> %s", original);
	} else if(strcmp(head, PROT_CON_R) == 0) {
		fprintf(stdout, "> You are now connected\n");
		pthread_cond_signal(&condition);
	} else if(strcmp(head, PROT_ERR) == 0) {
		fprintf(stdout, "> Server error : %s\n", tail);
		//temporaire, ie gérer les cas d'erreurs?
		quitter(args);
	} else if(strcmp(head, PROT_QUI_R) == 0) { 
		fprintf(stdout, "> OK for disconnect!\n");
		quitter(args);
	} else if(strcmp(head, PROT_LST_R) == 0) {
		fprintf(stdout, "> %s", tail);
	} else {
		fprintf(stderr, "Unrecognized : %s\n", original);
	}

	if((after = strtok(NULL, "\0")) != NULL)
		deformatage(args, after);
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
		if(strcmp(head, "quit") == 0 || strcmp(head, "exit") == 0) {
			send_msg(args, PROT_QUI);
		} else if(strcmp(original, "listu") == 0) {
			send_msg(args, PROT_LST);
		} else {
			fprintf(stdout, "\r> Command not recognized : %s\n", original);
		}
	}
}