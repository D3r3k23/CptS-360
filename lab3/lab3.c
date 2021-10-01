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

typedef enum
{
    RD_NONE,
    RD_STDIN,
    RD_STDOUT_TRUNC,
    RD_STDOUT_APPEND
} Redirect;

void exec_command(char* cmd, char* args[], int nArgs, char* envp[], Redirect redirect, const char* rd_pathname);
void exec        (char* cmd, char* args[], int nArgs, char* envp[], Redirect redirect, const char* rd_pathname);

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

        int currentCmdIndex = 0;
        for (int i = 0; i < nArgs; i++)
        {
            Redirect redirect = RD_NONE;
            if      (strcmp(args[i], "<")  == 0) redirect = RD_STDIN;
            else if (strcmp(args[i], ">")  == 0) redirect = RD_STDOUT_TRUNC;
            else if (strcmp(args[i], ">>") == 0) redirect = RD_STDOUT_APPEND;
            
            if (redirect != RD_NONE)
            {
                if (i == currentCmdIndex)
                {
                    printf("Enter a command before %s\n", args[i]);
                    break;
                }
                else if (!args[i + 1])
                {
                    printf("Enter a file name following %s\n", args[i]);
                    break;
                }
                else    
                {
                    LOG("Redirect %d to %s", redirect, args[i + 1]);
                    char* pathName = args[i + 1];
                    char* cmd      = args[currentCmdIndex];
                    char** cmdArgs = args + currentCmdIndex;
                    int nCmdArgs   = i - currentCmdIndex;
                    
                    exec_command(cmd, cmdArgs, nCmdArgs, envp, redirect, pathName);
                    i++;
                    currentCmdIndex = i + 1;
                }
            }
            else if (i == nArgs - 1)
            {
                char* cmd      = args[currentCmdIndex];
                char** cmdArgs = args + currentCmdIndex;
                int nCmdArgs   = i - currentCmdIndex + 1;

                exec_command(cmd, cmdArgs, nCmdArgs, envp, RD_NONE, NULL);
                break;
            }
        }
    }
}

void exec_command(char* cmd, char* args[], int nArgs, char* envp[], Redirect redirect, const char* rd_pathname)
{
    LOG("Executing cmd %s with %d args, redirect %s", cmd, nArgs, rd_pathname);
    for (int i = 0; i < nArgs; i++)
        LOG("arg[%d] = %s", i, args[i]);
        
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
            char* cmdArgs[32];
            for (int i = 0; i < nArgs; i++)
                cmdArgs[i] = args[i];
            cmdArgs[nArgs] = NULL;
            exec(cmd, args, nArgs, envp, redirect, rd_pathname);
        }
    }
}

void exec(char* cmd, char* args[], int nArgs, char* envp[], Redirect redirect, const char* rd_pathname)
{
    LOG("Executing cmd %s with %d args, redirect %s", cmd, nArgs, rd_pathname);
    for (int i = 0; i < nArgs; i++)
        LOG("arg[%d] = %s", i, args[i]);
    
    char* headCmd = cmd;
    char* headArgs[32];
    for (int i = 0; i < nArgs; i++)
        headArgs[i] = args[i];
    headArgs[nArgs] = NULL;
    int nHeadArgs = nArgs;

    char* tailCmd;
    char** tailArgs;
    int nTailArgs;

    int piped = 0;
    for (int i = 0; i < nArgs; i++)
        if (strcmp(args[i], "|") == 0)
        {
            piped = 1;
            tailCmd   = args[i + 1];
            tailArgs  = args + i + 1;
            nTailArgs = nArgs - i - 1;

            nHeadArgs = nArgs - nTailArgs - 1;
            headArgs[nHeadArgs] = NULL;
            break;
        }
    int pid;
    int pd[2];
    if (piped)
    {
        pid = fork();
        pipe(pd);
    }

    if (!piped || pid) // Parent
    {
        if (piped)
        {
            close(pd[0]);
            close(1);
            dup(pd[1]);
            close(pd[1]);
        }
        char pathname[128] = "\0";
        int found = 0;
        if (is_path(headCmd) && access(headCmd, F_OK) == 0)
        {
            found = 1;
            strcpy(pathname, headCmd);
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
                        if (strcmp(file->d_name, headCmd) == 0)
                        {
                            found = 1;
                            strcpy(pathname, dirName);
                            strcat(pathname, "/");
                            strcat(pathname, headCmd);
                        }
                }
            }
        }
        if (found)
        {
            switch (redirect)
            {
                case RD_STDIN:
                    close(0);
                    open(rd_pathname, O_RDONLY);
                    break;
                case RD_STDOUT_TRUNC:
                    close(1);
                    open(rd_pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    break;
                case RD_STDOUT_APPEND:
                    close(1);
                    open(rd_pathname, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    break;
            }

            execve(pathname, headArgs, envp);
            {
                LOG("execve failed: errno = %d", errno); // If stdout was closed?
                exit(errno);
            }
        }
        else
        {
            printf("Command \"%s\" not found\n", cmd);
            exit(-1);
        }
    }
    else // Child
    {
        if (piped)
        {
            close(pd[1]);
            close(0);
            dup(pd[0]);
            close(pd[0]);
        }
        exec(tailCmd, tailArgs, nTailArgs, envp, redirect, rd_pathname);
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
