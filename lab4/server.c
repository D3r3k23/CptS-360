#include "commands.h"

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

#define MAX 256
#define PORT 1234

enum { EXIT_SERVER = 78 };

void exec_cmd(char* line);

void get(const char* pathname);
void put(const char* pathname);

int cfd;

int main() 
{ 
    int sfd;
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
        exit(1);
    }

    // Now server is ready to listen and verification 
    if ((listen(sfd, 5)) != 0)
    { 
        printf("Listen failed\n");
        close(sfd);
        exit(1);
    }
    
    while (1)
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
        while (1)
        {
            printf("server: ready for next request ....\n");

            char message[MAX];
            int n = read(cfd, message, MAX);
            printf("server: read %d bytes, message: %s\n", n, message);
            exec_cmd(message);
        }
    }
    close(sfd);
}

void exec_cmd(char* line)
{
    int pd[2]; // 0: write | 1: read
    pipe(pd);

    int pid = fork();
    if (pid) // Parent proc
    {
        int status;
        pid = wait(&status);
        if (status == EXIT_SERVER)
        {
            close(cfd);
            exit(0);
        }

        close(pd[0]);

        char buf[MAX];
        read(pd[1], buf, MAX);
        
        int n = write(cfd, buf, MAX);
        printf("server: write %d bytes; response:\n%s", n, buf);
    }
    else // Child proc
    {
        char cwd[128];
        chroot(getcwd(cwd, 128));

        close(pd[1]);
        dup2(pd[0], 1); // Redirect stdout to pd[0]

        char* cmd_name = strtok(line, " ");
        char* pathname = strtok(NULL, " ");
        CMD cmd = find_cmd(cmd_name);

        switch (cmd)
        {
            case EXIT:
                exit(EXIT_SERVER);

            case GET:
                get(pathname);
                break;
            case PUT:

                break;

            case LS:    c_ls(pathname);    break;
            case CD:    c_cd(pathname);    break;
            case PWD:   c_pwd();           break;
            case MKDIR: c_mkdir(pathname); break;
            case RMDIR: c_rmdir(pathname); break;
            case RM:    c_rm(pathname);    break;
        }
    }
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

void get(const char* pathname)
{
    struct stat st;
    int r = lstat(pathname, &st);
    if (r == -1)
    {
        printf("Unable to open %s\n", pathname);
        return;
    }

    int file_size = st.st_size;
    char fs_str[16];
    sprintf(fs_str, "%d", file_size);
    write(cfd, fs_str, 16);

    int fd = open(pathname, O_RDONLY);
    if (fd != 0)
    {
        int total = 0;
        while (total < file_size)
        {
            char buf[MAX];
            int packet_size = min(file_size - total, MAX);
            int n = read(fd, buf, packet_size);
            total += n;
            write(cfd, buf, n);
        }
    }
}

void put(const char* pathname)
{

}
