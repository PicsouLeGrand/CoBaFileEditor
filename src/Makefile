CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lpthread
CURSES=-lncurses
EXEC=serveur client
LOG=logs/log.txt

all: $(EXEC)

serveur: serv/serveur_main.o serv/serveur_header.h serv/serveur_formatage.c
	$(CC) -o $@ $^ $(LIBS)

client: clnt/client_main.o clnt/client_header.h clnt/client_formatage.c
	$(CC) -o $@ $^ $(LIBS) $(CURSES)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf serv/*.o clnt/*.o

vanish:
	rm -rf $(EXEC) $(LOG)
