CC=gcc
CFLAGS=-Wall -Wextra -Werror -lpthread
EXEC=serveur client

all: $(EXEC)

serveur: serv/serveur_main.o header.h
	$(CC) -o $@ $^

client: clnt/client_main.o header.h
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o

mrproper:
	rm -rf $(EXEC)
