CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lpthread
EXEC=serveur client

all: $(EXEC)

serveur: serv/serveur_main.o serv/serveur_header.h serv/serveur_formatage.c
	$(CC) -o $@ $^ $(LIBS)

client: clnt/client_main.o clnt/client_header.h clnt/client_formatage.c
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf serv/*.o clnt/*.o

vanish:
	rm -rf $(EXEC)
