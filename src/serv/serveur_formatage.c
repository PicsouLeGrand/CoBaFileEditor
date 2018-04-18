/*
 * THIS FILE CONTAINS METHODS TO USE THE PROTOCOL EFFICIENTLY
 * Some of them create messages and some trunc messages for example
 */

#include "serveur_header.h"

/*
 * Format a string to write in the log file
 */
void write_to_log(struct client c, char *msg){
	char *log_string;
	char *port_c;

	log_string = malloc(BUFF_SIZE_RECV * sizeof(char));
	port_c = malloc(10*sizeof(char));
	
	strcat(log_string, msg);
	strcat(log_string, " ");
	strcat(log_string, c.ip);
	strcat(log_string, " ");
	sprintf(port_c, "%d", c.port);
	strcat(log_string, port_c);
	strcat(log_string, "\n");
	write(fd, log_string, strlen(log_string));
}

/*
 * Send a welcome message
 */
int send_welco(struct thread_args *args){
	char *welcome = "Welcome to the CoBa File Editor.\n> Type help for help.\n";
	return send(args->sock2, welcome, strlen(welcome) * sizeof(char), 0) < 0;
}

/*
 * Send a generic message
 */
int send_msg(struct thread_args *args, char *msg){
	return send(args->sock2, msg, strlen(msg) * sizeof(char), 0);
}

/*
 * Send an error message
 */
int send_err(int sock, char *message){
	char *error;
	error = malloc(BUFF_SIZE_ERR * sizeof(char));
	strcat(error, PROT_ERR);
	strcat(error, " ");
	strcat(error, message);

	return send(sock, error, strlen(error) * sizeof(char), 0);
}

/*
 * read a message from the socket and then act accordingly
 */
void deformatage(struct thread_args *args){
	int received;
	int is_connected = 0;
	char buff[BUFF_SIZE_RECV];
	
	received = recv(args->sock2, buff, 99*sizeof(char), 0);
	buff[received] = '\0';

	if(strcmp(buff, "") != 0){
		//first of all, client need to send con? request
		if(is_connected == 0){
			if(strcmp(buff, PROT_CON) == 0){
				//TODO verifs pour que le client se connecte bien
				//send confirmation message for connection
				send_msg(args, PROT_CON_R);
				is_connected = 1;
			}
		} else {
			//not supposed to happen
		}
		
		if(strcmp(buff, PROT_PNG_R) == 0){
			//printf("> Ping response from : %s and port : %d\n", inet_ntoa(args->caller.sin_addr),
			//args->caller.sin_port);
			clients[args->c.id].unanswered_pings = 0;
		} else if(strcmp(buff, PROT_CON) == 0) {
			printf("> %s | %d --> registration\n", args->c.ip, args->c.port);
			
			//for log purposes
			write_to_log(args->c, buff);
		} else if(strcmp(buff, PROT_QUI) == 0) {
			printf("> %s | %d --> disconnection\n", args->c.ip, args->c.port);
			send_msg(args, PROT_QUI_R);
			remove_client(args->c);
			
			//for log purposes
			write_to_log(args->c, buff);
		} else {
			printf("> %s from : %s and port : %d\n", buff, args->c.ip, args->c.port);
		}
	}
}