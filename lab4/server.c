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

void exec_cmd(const char* line);

int main() 
{ 
    int sfd, cfd, len;
    struct sockaddr_in saddr, caddr; 
    int length;
    
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
        exit(1);
    }
      
    // Now server is ready to listen and verification 
    if ((listen(sfd, 5)) != 0)
    { 
        printf("Listen failed\n"); 
        exit(1);
    }
    
    while (1)
    {
        // Try to accept a client connection as descriptor newsock
        length = sizeof(caddr);
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
        while (1)
        {
            printf("server: ready for next request ....\n");

            char message[MAX];
            int n = read(cfd, message, MAX);
            if (n == 0)
            {
                printf("server: client died, server loops\n");
                close(cfd);
                break;
            }
            printf("server: read  n=%d bytes; message=%s\n", n, message);

            char* cmd_name;
            char* pathname;
            sscanf(message, "%s%s", cmd_name, pathname);

            exec_cmd(message);

            char response[MAX];
            sprintf(response, "cmd=%s pathname=%s\n", cmd_name, pathname);

            // send the response to client 
            n = write(cfd, response, MAX);
            printf("server: wrote n=%d bytes; response=%s\n", n, response);
        }
    }
}

void exec_cmd(const char* line)
{

}
