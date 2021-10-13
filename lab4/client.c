#include "commands.h"

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

void client_command(const char* line);
void server_command(const char* line);

int sfd;

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
    while (1)
    {
        printf("Enter a command: ");
        char line[MAX];
        fgets(line, MAX, stdin);

        client_command(line);
    }
}

void client_command(const char* line)
{
    char* cmd_name;
    char* pathname;
    sscanf(line, "%s %s", cmd_name, pathname);
    CMD cmd = find_cmd(cmd_name);

    switch (cmd)
    {
        case GET:

            break;
        case PUT:

            break;

        case LS:
        case CD:
        case PWD:
        case MKDIR:
        case RMDIR:
        case RM:
            server_command(line);
            break;

        case LCAT:   c_cat(pathname);   break;
        case LLS:    c_ls(pathname);    break;
        case LCD:    c_cd(pathname);    break;
        case LPWD:   c_pwd();           break;
        case LMKDIR: c_mkdir(pathname); break;
        case LRMDIR: c_rmdir(pathname); break;
        case LRM:    c_rm(pathname);    break;

        default:
            return;
    }
}

void server_command(const char* line)
{
    int n;
    // Send message to server
    n = write(sfd, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);

    // Get response from server
    // Read a line from sock and show it
    char response[MAX];
    n = read(sfd, response, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, response);
}
