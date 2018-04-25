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
	memset(error, 0, BUFF_SIZE_ERR * sizeof(char));
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
	int i;
	char buff[BUFF_SIZE_RECV];
	char *head;
	char *tail;
	char *original;
	char *liste;
	char *nb_clients;
	char *info_client;
	char *port;
	char *path;
	char *list_files;
	DIR *d;
	struct dirent *dir;

	received = recv(args->sock2, buff, 99*sizeof(char), 0);
	if(received == -1)
		perror("received");

	buff[received] = '\0';

	original = malloc(strlen(buff) * sizeof(char) + 1);
	strcpy(original, buff);
	head = strtok(buff, " ");
	tail = strtok(NULL, "\n");

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
		} else if(strcmp(buff, PROT_LST) == 0) {
			//for log purposes
			write_to_log(args->c, buff);

			printf("> %s | %d --> list of users\n", args->c.ip, args->c.port);

			liste = malloc(BUFF_SIZE_SMALL*sizeof(char));
			nb_clients = malloc(BUFF_SIZE_SMALL*sizeof(char));
			info_client = malloc(BUFF_SIZE_MEDIUM*sizeof(char));
			port = malloc(BUFF_SIZE_SMALL*sizeof(char));

			sprintf(nb_clients, "%d", NB_CLIENTS);
			strcat(liste, PROT_LST_R);
			strcat(liste, " Il y a ");
			strcat(liste, nb_clients);
			strcat(liste, " client(s)\n");
			strcat(liste, SPECIAL_SEPARATOR);
			
			send_msg(args, liste);
			
			for(i = 0; i < NB_CLIENTS; i++){
				memset(info_client, 0, BUFF_SIZE_MEDIUM*sizeof(char));
				strcat(info_client, PROT_LST_R);
				strcat(info_client, " ");
				strcat(info_client, clients[i].ip);
				strcat(info_client, " | ");
				sprintf(port, "%d", clients[i].port);
				strcat(info_client, port);
				strcat(info_client, "\n");
				strcat(info_client, SPECIAL_SEPARATOR);
				
				send_msg(args, info_client);
			}
		} else if(strcmp(buff, PROT_LFI) == 0) {
			//for log purposes
			write_to_log(args->c, buff);

			printf("> %s | %d --> list of files\n", args->c.ip, args->c.port);

			list_files = malloc(BUFF_SIZE_RECV*sizeof(char));

			d = opendir("files");
			if(d) {
				while((dir = readdir(d)) != NULL) {
					if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 && strncmp(dir->d_name, ".nfs", 4) != 0) {
						memset(list_files, 0, BUFF_SIZE_RECV*sizeof(char));
						strcat(list_files, PROT_LFI_R);
						strcat(list_files, " ");
						strcat(list_files, dir->d_name);
						strcat(list_files, "\n");
						strcat(list_files, SPECIAL_SEPARATOR);
						send_msg(args, list_files);
					}
				}
				closedir(d);
			}
		} else if(strcmp(head, PROT_CRE) == 0) {
			write_to_log(args->c, buff);
			printf("> %s | %d --> file creation\n", args->c.ip, args->c.port);

			path = malloc(BUFF_SIZE_RECV*sizeof(char));
			strcat(path, "files/");
			strcat(path, tail);
			if(open(path, O_RDWR | O_CREAT | O_EXCL, 0755) != -1)
				send_msg(args, PROT_CRE_R);
			else
				send_err(args->sock2, ERR_MSG_2);
		} else if(strcmp(head, PROT_DEL) == 0) {
			write_to_log(args->c, buff);
			printf("> %s | %d --> file deletion\n", args->c.ip, args->c.port);

			path = malloc(BUFF_SIZE_RECV*sizeof(char));
			strcat(path, "files/");
			strcat(path, tail);
			if(remove(path) != -1)
				send_msg(args, PROT_DEL_R);
			else
				send_err(args->sock2, ERR_MSG_3);
		} else {
			printf("> %s from : %s and port : %d\n", buff, args->c.ip, args->c.port);
			//for log purposes
			write_to_log(args->c, buff);
		}
	}
}