all: commands.o server client

commands.o: commands.c commands.h
	gcc -c -o commands.o commands.c

server: server.c commands.o
	gcc -o server server.c commands.o

client: client.c commands.o
	gcc -o client client.c commands.o

clean:
	rm commands.o server client
