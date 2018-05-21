/*
 * THIS FILE CONTAINS METHODS TO USE THE PROTOCOL EFFICIENTLY
 * Some of them create messages and some trunc messages for example
 */

#include "client_header.h"

int started = 0;
int row;
int col;
int x = 0;
int y = 0;
int line_number = 1;
int before;
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
	
	getmaxyx(stdscr, row, col);

	// printf("o %s\n", original);
	// if(tail != NULL) printf("t %s\n", tail);
	if(strcmp(head, "Welcome") == 0){
		fprintf(stdout, "> %s", original);
	} else if(strcmp(head, PROT_CON_R) == 0) {
		fprintf(stdout, "> You are now connected.\n");
		pthread_cond_signal(&condition);
	} else if(strcmp(head, PROT_ERR) == 0) {
		if(started){
			printw("> Server error : %s\n", tail);
			refresh();
		} else
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
		pthread_mutex_lock(&mutex2);
		if(s == NULL){
			s = newterm(NULL, stdout, stdin);
			raw();
			keypad(stdscr, TRUE);
			getmaxyx(stdscr, row, col);
			started = 1;
		}
		pthread_mutex_unlock(&mutex2);

		if(before == 0){
			clear();
			before = 1;
		} else
			refresh();

		if(strcmp(tail, SPECIAL_EOF) == 0){
			before = 0;
			line_number = 1;
		} else {
			printw("%d | %s", line_number, tail);
			line_number++;
		}

		getyx(stdscr, y, x);

	} else {
		// fprintf(stderr, "Unrecognized : %s\n", original);
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
	char *ninput;

	// pthread_cond_init(&condition2, NULL);
	pthread_mutex_init(&mutex2, NULL);
	getmaxyx(stdscr, row, col);

	strtok(input, "\n");
	strcat(input, "\0");

	ninput = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(ninput, 0, BUFF_SIZE_INPUT*sizeof(char));

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
				pthread_mutex_lock(&mutex2);
				if(s == NULL){
					s = newterm(NULL, stdout, stdin);
					raw();
					keypad(stdscr, TRUE);
					getmaxyx(stdscr, row, col);
					started = 1;
				}
				pthread_mutex_unlock(&mutex2);

				modify_file(args, tail);

				while(strcmp(ninput, "exit") != 0 && strcmp(ninput, "quit") != 0){
					move(y, 0); //move to the end of file print
					clrtoeol(); //clear the line
					refresh();
					before = 0;
					getstr(ninput); //get user input
					curses_deformatage(args, ninput);
					refresh();
				}

				
				endwin();
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
 * Manage the input from inside the file modification mode
 */
void curses_deformatage(struct thread_args *args, char *input){
	char *head;
	char *tail;
	char *after;
	char *original;
	char *msg;

	before = 1;

	msg = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(msg, 0, BUFF_SIZE_INPUT*sizeof(char));
	original = malloc(strlen(input) * sizeof(char) + 1);
	strcpy(original, input);
	head = strtok(input, " ");
	tail = strtok(NULL, " ");
	after = strtok(NULL, "\n");

	if(head != NULL) {
		if(strcmp(head, CURSES_CMD_DEL) == 0){
			if(tail != NULL){
				strcat(msg, CURSES_DEL);
				strcat(msg, " ");
				strcat(msg, tail);
				send_msg(args, msg);
				refresh();
			} else {
				printw("%s", ERR_MSG_4);
				refresh();
			}
		} else if(strcmp(head, CURSES_CMD_INS) == 0){
			strcat(msg, CURSES_INS);
			if(tail != NULL) {
				strcat(msg, " ");
				strcat(msg, tail);
			}
			send_msg(args, msg);
			refresh();
		} else if(strcmp(head, CURSES_CMD_MOD) == 0){
			if(tail == NULL || after == NULL){
				printw("%s", ERR_MSG_4);
				refresh();
			} else {
				strcat(msg, CURSES_MOD);
				strcat(msg, " ");
				strcat(msg, tail);
				strcat(msg, " ");
				strcat(msg, after);
				send_msg(args, msg);
			}
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
		"\t- %s, %s [filename] -> delete a file with name [filename]\n"
		"\t----------- While in Curses mode-------------\n"
		"\t- %s [line number] -> delete a line\n"
		"\t- %s (line number) -> insert a line at (line number), or at the end\n"
		"\t- %s [line number] [text]-> modify a line\n"
		"\t- %s -> quit Curses mode\n",
		CMD_HELP, CMD_HELP_SHORT, CMD_EXIT, CMD_QUIT, CMD_LSTU, CMD_LSTU_SHORT,
		CMD_LSTF, CMD_LSTF_SHORT, CMD_CREA, CMD_CREA_SHORT,
		CMD_MODI, CMD_MODI_SHORT, CMD_DELE, CMD_DELE_SHORT,
		CURSES_CMD_DEL, CURSES_CMD_INS, CURSES_CMD_MOD, CMD_QUIT);
}

/*
 * Format a message to create a file
 */
void create_file(struct thread_args *args, char *name){
	char *message = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(message, 0, BUFF_SIZE_INPUT*sizeof(char));
	strcat(message, PROT_CRE);
	strcat(message, " ");
	strcat(message, name);

	send_msg(args, message);
}

/*
 * Format a message to modify a file
 */
void modify_file(struct thread_args *args, char *name){
	char *crow = malloc(10*sizeof(char));
	char *message = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(message, 0, BUFF_SIZE_INPUT*sizeof(char));
	memset(crow, 0, 10*sizeof(char));

	strcat(message, PROT_MOD);
	strcat(message, " ");
	strcat(message, "files/");
	strcat(message, name);
	strcat(message, " ");
	sprintf(crow, "%d", row - 1); //-1 to give room for user input afterwards
	strcat(message, crow);

	send_msg(args, message);
}

/*
 * Format a message to modify a file
 */
void delete_file(struct thread_args *args, char *name){
	char *message = malloc(BUFF_SIZE_INPUT*sizeof(char));
	memset(message, 0, BUFF_SIZE_INPUT*sizeof(char));
	strcat(message, PROT_DEL);
	strcat(message, " ");
	strcat(message, name);

	send_msg(args, message);
}