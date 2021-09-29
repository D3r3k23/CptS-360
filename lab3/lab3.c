#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <unistd.h>
#include <sys/wait.h>

#ifdef LOG // gcc -DLOG to enable
    #undef LOG
    // Forwards __VA_ARGS__ to printf
    #define LOG(...)                             \
        do {                                     \
            printf("[LOG] <%s> ", __FUNCTION__); \
            printf(__VA_ARGS__);                 \
            printf("\n");                        \
        } while (0)
#else
    #define LOG(...)
#endif

void exec_command(char* cmd, char* args[], int nArgs, char* envp[]);

int tokenize(char* src, char* token, char* dest[], int maxCount);
int is_path(char* path);

char* pathDirs[64];
int nPathDirs;

int main(int argc, char* argv[], char* envp[])
{
    pathDirs[0] = "\0";
    char pathStr[1024];
    strcpy(pathStr, getenv("PATH"));
    nPathDirs = tokenize(pathStr, ":", pathDirs, 64);

    for (int i = 0; i < nPathDirs; i++)
        LOG("PATH[%d] = %s", i, pathDirs[i]);

    while (1)
    {
        char* cwd = basename(getenv("PWD"));
        printf("sh[pid=%d]:%s$ ", getpid(), cwd);

        char line[128];
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = '\0'; // Trim newline

        char* args[32];
        int nArgs = tokenize(line, " ", args, 32);
        if (nArgs <= 0)
            continue;

        for (int i = 0; i < nArgs; i++)  
            LOG("arg[%d] = %s", i, args[i]);

        exec_command(args[0], args, nArgs, envp);
    }
}

void exec_command(char* cmd, char* args[], int nArgs, char* envp[])
{
    if (strcmp(cmd, "cd") == 0)
    {
        char* newCWD = (nArgs > 1) ? args[1] : getenv("HOME");
        chdir(newCWD);
    }
    else if (strcmp(cmd, "exit") == 0)
    {
        int exitCode = (nArgs > 1) ? atoi(args[1]) : 0;
        exit(0);
    }
    else
    {
        int pid = fork();
        
        if (pid) // In parent
        {
            printf("Process %d forked child %d\n", getpid(), pid);
            int status;
            pid = wait(&status);
            LOG("ZOMBIE child=%d exitStatus=%x", pid, status); 
        }
        else // In child
        {
            printf("Process %d running\n", getpid());
            char pathname[128] = "\0";
            int found = 0;
            if (is_path(cmd))
            {
                if (access(cmd, F_OK) == 0)
                {
                    found = 1;
                    strcpy(pathname, cmd);
                }
            }
            else
            {
                for (int i = 0; !found && (i < nPathDirs); i++)
                {
                    char* dirName = pathDirs[i];
                    DIR* dir;
                    if (dir = opendir(dirName))
                    {
                        struct dirent* file;
                        while (!found && (file = readdir(dir)))
                            if (strcmp(file->d_name, cmd) == 0)
                            {
                                found = 1;
                                strcpy(pathname, dirName);
                                strcat(pathname, "/");
                                strcat(pathname, cmd);
                            }
                    }
                }
            }
            if (found)
            {
                execve(pathname, args, envp);
                {
                    LOG("execve failed: errno = %d", errno);
                    exit(errno);
                }
            }
            else
            {
                printf("Command \"%s\" not found\n", cmd);
                exit(-1);
            }
        }
    }
}

int tokenize(char* src, char* token, char* dest[], int maxCount)
{
    int n = 0;
    for (char* s = strtok(src, token); s; s = strtok(NULL, token))
    {
        if (n == maxCount - 1)
        {
            dest[n] = NULL;
            return n;
        }
        else
            dest[n++] = s;
    }
    dest[n] = NULL;
    return n;
}

int is_path(char* path)
{
    char temp[64];
    strcpy(temp, path);
    char* base = basename(temp);
    return strcmp(path, base) != 0;
}

/********************* YOU DO ***********************
1. I/O redirections:

Example: line = arg0 arg1 ... > argn-1

  check each arg[i]:
  if arg[i] = ">" {
     arg[i] = 0; // null terminated arg[ ] array 
     // do output redirection to arg[i+1] as in Page 131 of BOOK
  }
  Then execve() to change image


2. Pipes:

Single pipe   : cmd1 | cmd2 :  Chapter 3.10.3, 3.11.2

Multiple pipes: Chapter 3.11.2
****************************************************/

    
