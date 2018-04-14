/*
 * THIS FILE CONTAINS METHODS TO USE THE PROTOCOL EFFICIENTLY
 * Some of them create messages and some trunc messages for example
 */

#include "serveur_header.h"

int send_welco(struct thread_args *args){
	char *welcome = "Welcome to the CoBa File Editor.\nType help for help.\n";
	return send(args->sock2, welcome, strlen(welcome) * sizeof(char), 0) < 0;
}

int send_con_r(struct thread_args *args){
	return send(args->sock2, PROT_CON_R, strlen(PROT_CON_R) * sizeof(char), 0);
}

