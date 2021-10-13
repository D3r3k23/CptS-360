gcc -o commands.o commands.c
gcc -o server server.c commands.o
gcc -o client client.c commands.o
