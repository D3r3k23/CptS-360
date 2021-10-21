#include "commands.h"
#include "file_transfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h> 

#define MAX 256
#define PORT 1234

void client_command(char* line);
void server_command(char* line);

int sfd;
int running = 1;

int main(int argc, char *argv[], char *env[]) 
{ 
    char how[64];

    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0)
    { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 

    printf("********  client processing loop  *********\n");
    while (running)
    {
        char cwd[128];
        const char* dirname = basename(getcwd(cwd, 128));
        printf("client:%s$ ", dirname);

        char line[MAX];
        fgets(line, MAX, stdin);
        line[strlen(line) - 1] = '\0'; // Trim newline

        client_command(line);
    }
    close(sfd);
}

void client_command(char* line)
{
    char temp[MAX];
    strcpy(temp, line);
    char* cmd_name = strtok(temp, " ");
    char* pathname = strtok(NULL, " ");
    
    CMD cmd = find_cmd(cmd_name);
    printf("Cmd: %s\n", cmd_name);
    printf("Pathname: %s\n", pathname);

    switch (cmd)
    {
        // Exit client and server
        case EXIT:
            server_command("exit");
            running = 0;
            break;

        // 1. Send command
        // 2. Receive file
        case GET:
            server_command(line);
            recv_file(sfd, pathname);
            break;

        // 1. Send command
        // 2. Send file
        case PUT:
            server_command(line);
            send_file(sfd, pathname);
            break;

        // 1. Send command
        // 2. Receive and delete file
        case LS:
        case PWD:
            server_command(line);
            recv_file(sfd, ".temp.txt");
            c_cat(".temp.txt", stdout);
            c_rm(".temp.txt");
            break;

        // Send command to server
        case CD:
        case MKDIR:
        case RMDIR:
        case RM:
            server_command(line);
            break;

        // Execute command locally
        case LCAT:   c_cat(pathname, stdout); break;
        case LLS:    c_ls(pathname, stdout);  break;
        case LCD:    c_cd(pathname);          break;
        case LPWD:   c_pwd(stdout);           break;
        case LMKDIR: c_mkdir(pathname);       break;
        case LRMDIR: c_rmdir(pathname);       break;
        case LRM:    c_rm(pathname);          break;
    }
}

void server_command(char* line)
{
    int n = write(sfd, line, MAX);
    printf("Write %d bytes, message: %s\n", n, line);
}
