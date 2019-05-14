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
	time_t today;
	struct tm date;
	char *string_date;

	time(&today);
	date = *localtime(&today);
	string_date = asctime(&date);

	log_string = malloc(BUFF_SIZE_RECV * sizeof(char));
	memset(log_string, 0, BUFF_SIZE_RECV * sizeof(char));
	port_c = malloc(10*sizeof(char));

	strcat(log_string, string_date);
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
 * Send a modified file back to the user, uses the user terminal height to
 * send the appropriate ammount of lines from the file
 */
void modification(struct thread_args *args, char *tail, char *after){
	int fd;
	int rows;
	char *c;
	char *line;
	char *msg;
	char *path;

	args->c.nb_open_files++;
	args->c.is_modifying++;
	strcpy(args->c.height, after);

	c = malloc(sizeof(char));
	msg = malloc(BUFF_SIZE_RECV*sizeof(char));
	line = malloc(BUFF_SIZE_RECV*sizeof(char));
	path = malloc(BUFF_SIZE_RECV*sizeof(char));

	rows = atoi(after);

	memset(msg, 0, sizeof(msg));
	memset(line, 0, sizeof(line));

	strcpy(args->c.file, tail);

	if((fd = open(tail, O_RDWR, 0755)) != -1){
		while (read(fd, c, 1) != 0 && rows != 0){
			strcat(line, c);
			if(strcmp(c,"\n") == 0) {
				strcat(msg, PROT_MOD_R);
				strcat(msg, " ");
				strcat(msg, line);
				strcat(msg, SPECIAL_SEPARATOR);
				send_msg(args, msg);
				memset(msg, 0, sizeof(msg));
				memset(line, 0, sizeof(line));
				rows--;
			}
		}
		strcat(msg, PROT_MOD_R);
		strcat(msg, " ");
		strcat(msg, SPECIAL_EOF);
		send_msg(args, msg);

	} else {
		perror("open mod");
		send_err(args->sock2, ERR_MSG_4);
	}
}

/*
 * Send the list of current users
 */
void liste_users(struct thread_args *args, char *buff){
	char *liste;
	char *nb_clients;
	char *info_client;
	char *port;
	int i;

	//for log purposes
	write_to_log(args->c, buff);

	printf("> %s | %d --> list of users\n", args->c.ip, args->c.port);

	liste = malloc(BUFF_SIZE_SMALL*sizeof(char));
	nb_clients = malloc(BUFF_SIZE_SMALL*sizeof(char));
	info_client = malloc(BUFF_SIZE_RECV*sizeof(char));
	port = malloc(BUFF_SIZE_SMALL*sizeof(char));

	sprintf(nb_clients, "%d", NB_CLIENTS);
	memset(liste, 0, BUFF_SIZE_SMALL*sizeof(char));
	strcat(liste, PROT_LST_R);
	strcat(liste, " Il y a ");
	strcat(liste, nb_clients);
	strcat(liste, " client(s)\n");
	strcat(liste, SPECIAL_SEPARATOR);

	send_msg(args, liste);

	for(i = 0; i < NB_CLIENTS; i++){
		memset(info_client, 0, BUFF_SIZE_RECV*sizeof(char));
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
}

/*
 * Send the list of current files
 */
void liste_files(struct thread_args *args, char *buff){
	char *list_files;
	DIR *d;
	struct dirent *dir;

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
}

/*
 * Used to delete a line from a file when in ncurses mode
 */
void curses_line_delete(struct thread_args *args, char *buff, char *tail){
	int replica_fd;
	int path_fd;
	int line_number;
	char *c;
	char *replica = "replica.c";
	int temp = 1;

	c = malloc(sizeof(char));

	line_number = strtol(tail, NULL, 10);

	write_to_log(args->c, buff);
	printf("> %s | %d ==> line deletion [CURSES]\n", args->c.ip, args->c.port);


	pthread_mutex_lock(&mutex);
	//open the file for replication
	if((replica_fd = open(replica, O_RDWR | O_CREAT | O_EXCL, 0755)) == -1){
		perror("open curses");
	}
	//open the original file
	if((path_fd = open(args->c.file, O_RDONLY, 0755)) == -1){
		perror("open curses 2");
	}

	//copy the content of path into replica, without the line deleted
	while(read(path_fd, c, 1) != 0){
		if(temp != line_number)
			write(replica_fd, c, 1);
		if(strcmp(c,"\n") == 0)
			temp++;
	}

	close(replica_fd);
	close(path_fd);
	remove(args->c.file);
	rename(replica, args->c.file);

	pthread_mutex_unlock(&mutex);
	send_modifs_to_all(args);

	//quand on supprime un fichier on le supprime de la liste des autres
}

/*
 * Used to insert a blank line into a file when in ncurses mode
 */
void curses_line_insert(struct thread_args *args, char *buff, char *tail){
	int replica_fd;
	int path_fd;
	int line_number = -1;
	char *c;
	char *sentence;
	char *replica = "replica.c";
	int temp = 1;

	c = malloc(sizeof(char));
	sentence = malloc(BUFF_SIZE_RECV*sizeof(char));

	if(tail != NULL)
		line_number = strtol(tail, NULL, 10);

	write_to_log(args->c, buff);
	printf("> %s | %d ==> line insertion [CURSES]\n", args->c.ip, args->c.port);


	pthread_mutex_lock(&mutex);
	//open the file for replication
	if((replica_fd = open(replica, O_RDWR | O_CREAT | O_EXCL, 0755)) == -1){
		perror("open curses");
	}
	//open the original file
	if((path_fd = open(args->c.file, O_RDONLY, 0755)) == -1){
		perror("open curses 2");
	}

	//copy the content of path into replica, without the line deleted
	while(read(path_fd, c, 1) != 0){
		strcat(sentence, c);
		if(temp != line_number)
			write(replica_fd, c, 1);
		else {
			if(strcmp(c, "\n") == 0){
				strcat(sentence, "\n");
				write(replica_fd, sentence, strlen(sentence));
			}
		}
		if(strcmp(c,"\n") == 0){
			memset(sentence, 0, BUFF_SIZE_RECV*sizeof(char));
			temp++;
		}
	}

	//if tail is null then insert at the end of file
	if(tail == NULL)
		write(replica_fd, "\n", 1);

	close(replica_fd);
	close(path_fd);
	remove(args->c.file);
	rename(replica, args->c.file);

	pthread_mutex_unlock(&mutex);
	send_modifs_to_all(args);

	//delete the line
	//tell others that the line is deleted
	//quand on supprime un fichier on le supprime de la liste des autres
}

/*
 * Used to insert a line into a file when in ncurses mode
 */
void curses_line_modification(struct thread_args *args, char *buff, char *tail, char *after){
	int replica_fd;
	int path_fd;
	int line_number;
	char *c;
	char *replica = "replica.c";
	int temp = 1;

	c = malloc(sizeof(char));

	line_number = strtol(tail, NULL, 10);

	write_to_log(args->c, buff);
	printf("> %s | %d ==> line modification [CURSES]\n", args->c.ip, args->c.port);


	pthread_mutex_lock(&mutex);
	//open the file for replication
	if((replica_fd = open(replica, O_RDWR | O_CREAT | O_EXCL, 0755)) == -1){
		perror("open curses");
	}
	//open the original file
	if((path_fd = open(args->c.file, O_RDONLY, 0755)) == -1){
		perror("open curses 2");
	}

	//copy the content of path into replica, with the line modified
	while(read(path_fd, c, 1) != 0){
		if(temp != line_number)
			write(replica_fd, c, 1);
		else {
			if(strcmp(c, "\n") == 0){
				write(replica_fd, after, strlen(after));
				write(replica_fd, "\n", 1);
			}
		}

		if(strcmp(c,"\n") == 0)
			temp++;
	}

	close(replica_fd);
	close(path_fd);
	remove(args->c.file);
	rename(replica, args->c.file);

	pthread_mutex_unlock(&mutex);
	send_modifs_to_all(args);
}

/*
 * For each client modifying the file, send them the modified file
 */
void send_modifs_to_all(struct thread_args *args){
	int i;
	char *path;

	path = malloc(BUFF_SIZE_RECV*sizeof(char));

	strcpy(path, args->c.file);
	strtok(path, "/");
	path = strtok(NULL, "");
	//modification(args, path, args->c.height);

	for(i = 0; i < NB_CLIENTS; i++){
		if(strcmp(args->c.file, clients[i].file) == 0){
			modification(global_args[i], clients[i].file, clients[i].height);
		}
	}
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
	char *after;
	char *original;
	char *path;

	received = recv(args->sock2, buff, 99*sizeof(char), 0);
	if(received == -1)
		perror("received");

	buff[received] = '\0';

	original = malloc(strlen(buff) * sizeof(char) + 1);
	strcpy(original, buff);
	head = strtok(buff, " ");
	tail = strtok(NULL, " ");
	after = strtok(NULL, "\n");

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
			liste_users(args, buff);
		} else if(strcmp(buff, PROT_LFI) == 0) {
			liste_files(args, buff);
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
			if(remove(path) != -1){
				send_msg(args, PROT_DEL_R);
				//delete the file from all clients
				for(i = 0; i < MAX_CLIENTS; i++){
					if(strcmp(tail, clients[i].file) == 0)
						clients[i].file = NULL;
				}
			} else
				send_err(args->sock2, ERR_MSG_3);
		} else if(strcmp(head, PROT_MOD) == 0) {
			write_to_log(args->c, buff);
			printf("> %s | %d --> file modification\n", args->c.ip, args->c.port);
			modification(args, tail, after);
		} else if(strcmp(head, CURSES_DEL) == 0) {
			curses_line_delete(args, buff, tail);
		} else if(strcmp(head, CURSES_INS) == 0) {
			curses_line_insert(args, buff, tail);
		} else if(strcmp(head, CURSES_MOD) == 0) {
			curses_line_modification(args, buff, tail, after);
		} else {
			printf("> %s from : %s and port : %d\n", buff, args->c.ip, args->c.port);
			//for log purposes
			write_to_log(args->c, buff);
		}
	}
}
