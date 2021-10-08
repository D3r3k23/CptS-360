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
#include <libgen.h> // for dirname()/basename()
#include <time.h> 

#define MAX 256
#define PORT 1234

char line[MAX], ans[MAX];
int n;

struct sockaddr_in saddr; 
int sfd;

int main(int argc, char *argv[], char *env[]) 
{ 
    int n;
    char how[64];
    int i;

    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0)
    { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
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

    printf("********  processing loop  *********\n");
    while (1)
    {
        int a;
        printf("enter a: ");
        scanf("%d", &a);
        printf("\n");
        int b;
        printf("enter b: ");
        scanf("%d", &b);
        printf("\n");

        sprintf(line, "%d %d", a, b);

        // Send ENTIRE line to server
        n = write(sfd, line, MAX);
        printf("client: wrote n=%d bytes; line=%s\n", n, line);

        // Read a line from sock and show it
        n = read(sfd, ans, MAX);
        printf("client: read  n=%d bytes; echo=%s\n",n, ans);
    }
}

