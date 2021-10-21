all: commands.o file_transfer.o server client

commands.o: commands.c commands.h
	gcc -g -c -o commands.o commands.c

file_transfer.o: file_transfer.c file_transfer.h
	gcc -g -c -o file_transfer.o file_transfer.c

server: server.c commands.o file_transfer.o
	gcc -g -o server/server server.c commands.o file_transfer.o

client: client.c commands.o file_transfer.o
	gcc -g -o client/client client.c commands.o file_transfer.o

clean:
	rm commands.o file_transfer.o server/server client/client
