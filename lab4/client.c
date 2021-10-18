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

void client_command(char* line);
char* server_command(char* line);

void get(const char* pathname, int file_size);
void put(const char* pathname);

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
        char cwd[128];
        const char* dirname = basename(getcwd(cwd, 128));
        printf("[Client:%s] $", dirname);
        char line[MAX];
        fgets(line, MAX, stdin);

        client_command(line);
    }
    close(sfd);
}

void client_command(char* line)
{
    char* cmd_name = strtok(line, " ");
    char* pathname = strtok(NULL, " ");
    CMD cmd = find_cmd(cmd_name);

    char* response;
    switch (cmd)
    {
        case LEXIT:
            close(sfd);
            exit(0);

        case GET:
            response = server_command(line);
            get(pathname, atol(response));
            break;
        case PUT:
            server_command(line);
            put(pathname);
            break;

        case EXIT:
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
    }
}

char* server_command(char* line)
{
    // Send message to server
    int n = write(sfd, line, MAX);
    printf("client: write %d bytes, message: %s\n", n, line);

    // Get response from server
    static char response[MAX];
    bzero(response, MAX);
    n = read(sfd, response, MAX);
    printf("client: read %d bytes, response:\n%s", n, response);
    return response;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

void get(const char* pathname, int file_size)
{
    int fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd != -1)
    {
        int total = 0;
        while (total < file_size)
        {
            char buf[MAX];
            int packet_size = min(file_size - total, MAX);
            int n = read(sfd, buf, packet_size);
            total += n;
            write(fd, buf, n);
        }
        close(fd);
    }
}

void put(const char* pathname)
{

}
