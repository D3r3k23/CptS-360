#include "commands.h"
#include "file_transfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h>
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>

#define MAX 256
#define PORT 1234

void exec_cmd(char* line);

int cfd; // Client FD
int running = 1;

int main() 
{
    char cwd[128];
    chroot(getcwd(cwd, 128));

    int sfd; // Server FD
    struct sockaddr_in saddr, caddr;
    
    printf("1. Create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        printf("Socket creation failed\n");
        exit(1);
    }
    
    printf("2. Fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    // saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(PORT);
    
    printf("3. Bind socket to server\n");
    if ((bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr))) != 0)
    {
        printf("Socket bind failed\n");
        printf("errno[%d]: %s\n", errno, strerror(errno));
        exit(1);
    }

    // Now server is ready to listen and verification 
    if ((listen(sfd, 5)) != 0)
    { 
        printf("Listen failed\n");
        close(sfd);
        exit(1);
    }
    
    while (running)
    {
        // Try to accept a client connection as descriptor newsock
        int length = sizeof(caddr);
        cfd = accept(sfd, (struct sockaddr*)&caddr, &length);
        if (cfd < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }

        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop
        while (running)
        {
            printf("Server ready for next request ...\n");

            char message[MAX];
            int n = read(cfd, message, MAX);
            printf("Read %d bytes, message: %s\n", n, message);
            
            exec_cmd(message);
        }
        close(cfd);
    }
    close(sfd);
}

void exec_cmd(char* line)
{
    char* cmd_name = strtok(line, " ");
    char* pathname = strtok(NULL, " ");

    CMD cmd = find_cmd(cmd_name);
    printf("command:  %s\n", cmd_name);
    printf("pathname: %s\n", pathname);

    FILE* f;
    switch (cmd)
    {
        // Exit server
        case EXIT:
            running = 0;
            break;

        // Send requested file
        case GET:
            send_file(cfd, pathname);
            break;

        // Receive and save requested file
        case PUT:
            recv_file(cfd, pathname);
            break;

        // 1. Execute command, save output to file
        // 2. Send and delete file
        case LS:
            f = fopen(".ls.txt", "w");
            c_ls(pathname, f);
            fclose(f);
            send_file(cfd, ".ls.txt");
            c_rm(".ls.txt");
            break;
        case PWD:
            f = fopen(".pwd.txt", "w");
            c_pwd(f);
            fclose(f);
            send_file(cfd, ".pwd.txt");
            c_rm(".pwd.txt");
            break;

        // Silently execute command
        case CD:    c_cd(pathname);    break;
        case MKDIR: c_mkdir(pathname); break;
        case RMDIR: c_rmdir(pathname); break;
        case RM:    c_rm(pathname);    break;

        default:
            printf("Unknown command\n");
            break;
    }
}
