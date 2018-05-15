/*
 * THIS FILE CONTAINS METHODS TO USE THE PROTOCOL EFFICIENTLY
 * Some of them create messages and some trunc messages for example
 */

#include "client_header.h"

int started = 0;
SCREEN *s = NULL;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition2 = PTHREAD_COND_INITIALIZER;

/*
 * Send a message through the global TCP socket to the server
 */ 
void send_msg(struct thread_args *args, char *msg){
	char *res;

	res = malloc(BUFF_SIZE_INPUT*sizeof(char));
	strcpy(res, msg);
	res = strtok(res, "\n");
	strcat(res, "\0");

	if(write(args->sock, res, strlen(res) * sizeof(char)) == -1){
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
		fprintf(stdout, "> You are now connected.\n");
		pthread_cond_signal(&condition);
	} else if(strcmp(head, PROT_ERR) == 0) {
		fprintf(stderr, "> Server error : %s\n", tail);

		if(strcmp(tail, ERR_MSG_1) == 0)
			quitter(args);
	} else if(strcmp(head, PROT_QUI_R) == 0) { 
		quitter(args);
	} else if(strcmp(head, PROT_LST_R) == 0) {
		fprintf(stdout, "> %s", tail);
	} else if(strcmp(head, PROT_LFI_R) == 0) {
		fprintf(stdout, "> %s", tail);
	} else if(strcmp(head, PROT_CRE_R) == 0) {
		fprintf(stdout, "> The file was successfully created.\n");
	} else if(strcmp(head, PROT_DEL_R) == 0) {
		fprintf(stdout, "> The file was successfully deleted.\n");
	} else if(strcmp(head, PROT_MOD_R) == 0) {
		// pthread_mutex_lock(&mutex2);
		// pthread_cond_wait(&condition2, &mutex2);
		// pthread_mutex_unlock(&mutex2);

		pthread_mutex_lock(&mutex2);
		if(s == NULL){
			s = newterm(NULL, stdout, stdin);
			raw();
			keypad(stdscr, TRUE);
		}
		pthread_mutex_unlock(&mutex2);

		printw("%s", tail);
		refresh();
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

	// pthread_cond_init(&condition2, NULL);
	pthread_mutex_init(&mutex2, NULL);

	strtok(input, "\n");
	strcat(input, "\0");

	original = malloc(strlen(input) * sizeof(char) + 1);
	strcpy(original, input);
	head = strtok(input, " ");
	tail = strtok(NULL, "\n");

	if(head != NULL && strcmp(original, "\n") != 0 && strcmp(head, "\t") != 0){
		if(strcmp(head, CMD_QUIT) == 0 || strcmp(head, CMD_EXIT) == 0) {
			send_msg(args, PROT_QUI);
		} else if(strcmp(head, CMD_LSTU) == 0 || strcmp(head, CMD_LSTU_SHORT) == 0) {
			send_msg(args, PROT_LST);
		} else if(strcmp(head, CMD_HELP) == 0 || strcmp(head, CMD_HELP_SHORT) == 0) {
			print_help();
		} else if(strcmp(head, CMD_CREA) == 0 || strcmp(head, CMD_CREA_SHORT) == 0) {
			if(tail != NULL)
				create_file(args, tail);
			else
				fprintf(stderr, "%s", ERR_MSG_3);
		} else if(strcmp(head, CMD_MODI) == 0 || strcmp(head, CMD_MODI_SHORT) == 0) {
			if(tail != NULL) {
				modify_file(args, tail);

				pthread_mutex_lock(&mutex2);
				if(s == NULL){
					s = newterm(NULL, stdout, stdin);
					raw();
					keypad(stdscr, TRUE);
				}
				pthread_mutex_unlock(&mutex2);

				getch();
				clear();
				endwin();
				//delscreen(s);
				
			} else
				fprintf(stderr, "%s", ERR_MSG_3);
		} else if(strcmp(head, CMD_DELE) == 0 || strcmp(head, CMD_DELE_SHORT) == 0) {
			if(tail != NULL)
				delete_file(args, tail);
			else
				fprintf(stderr, "%s", ERR_MSG_3);
		} else if(strcmp(head, CMD_LSTF) == 0 || strcmp(head, CMD_LSTF_SHORT) == 0) {
			send_msg(args, PROT_LFI);
		} else {
			fprintf(stderr, "\r> Command not recognized : %s\n", original);
		}
	}
}

/* 
 * Display the help message
 */
void print_help(){
	fprintf(stdout,															"> List of available commands :\n"
		"\t- %s, %s -> display this help\n"
		"\t- %s, %s -> quit the program\n"
		"\t- %s, %s -> display a list of current users\n"
		"\t- %s, %s -> display a list of current files\n"
		"\t- %s, %s [filename] -> create a file with name [filename]\n"
		"\t- %s, %s [filename] -> modify a file with name [filename]\n"
		"\t- %s, %s [filename] -> delete a file with name [filename]\n",
		CMD_HELP, CMD_HELP_SHORT, CMD_EXIT, CMD_QUIT, CMD_LSTU, CMD_LSTU_SHORT,
		CMD_LSTF, CMD_LSTF_SHORT, CMD_CREA, CMD_CREA_SHORT,
		CMD_MODI, CMD_MODI_SHORT, CMD_DELE, CMD_DELE_SHORT);
}

void create_file(struct thread_args *args, char *name){
	char *message = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(message, 0, BUFF_SIZE_INPUT*sizeof(char));
	strcat(message, PROT_CRE);
	strcat(message, " ");
	strcat(message, name);

	send_msg(args, message);
}

void modify_file(struct thread_args *args, char *name){
	char *message = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(message, 0, BUFF_SIZE_INPUT*sizeof(char));
	strcat(message, PROT_MOD);
	strcat(message, " ");
	strcat(message, name);

	send_msg(args, message);
}

void delete_file(struct thread_args *args, char *name){
	char *message = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(message, 0, BUFF_SIZE_INPUT*sizeof(char));
	strcat(message, PROT_DEL);
	strcat(message, " ");
	strcat(message, name);

	send_msg(args, message);
}