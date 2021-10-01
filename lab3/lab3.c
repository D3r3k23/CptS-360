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

void exec_command_line(char* cmd, char* args[], int nArgs, char* envp[]);
void exec_pipe(char* cmd, char* args[], int nArgs, char* envp[], int pd[2]);
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

        exec_command_line(args[0], args, nArgs, envp);
    }
}

void exec_command_line(char* cmd, char* args[], int nArgs, char* envp[])
{
    LOG("Executing command line:");
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
        exit(exitCode);
    }
    else
    {
        int pid = fork();
        if (pid) // Parent proc
        {
            printf("Process %d forked child %d\n", getpid(), pid);
            int status;
            pid = wait(&status);
            LOG("ZOMBIE child=%d exitStatus=%x", pid, status);
        }
        else // Child process
        {
            exec_pipe(cmd, args, nArgs, envp, NULL);
        }
    }
}

void exec_pipe(char* cmd, char* args[], int nArgs, char* envp[], int pd[2])
{
    LOG("Executing command %s%s", cmd, (pd ? " with pipe:" : ":"));
    for (int i = 0; i < nArgs; i++)  
        LOG("arg[%d] = %s", i, args[i]);

    if (pd) // Pipe passed in
    {
        close(pd[0]);
        dup2(pd[1], 1);
        close(pd[1]);
    }

    int lastPipeIndex = 0;
    for (int i = 0; i < nArgs; i++)
        if (strcmp(args[i], "|") == 0)
            lastPipeIndex = i;
    
    if (lastPipeIndex != 0)
    {
        char* headCmd = cmd;
        char* headArgs[32];
        int nHeadArgs = nArgs - lastPipeIndex - 1;
        for (int i = 0; i < nHeadArgs; i++)
            headArgs[i] = args[i];
        headArgs[nHeadArgs] = NULL;

        char* tailCmd = args[lastPipeIndex + 1];
        char* tailArgs[32];
        int nTailArgs = nArgs - lastPipeIndex - 1;
        for (int i = 0; i < nTailArgs; i++)
            tailArgs[i] = args[i + lastPipeIndex + 1];
        tailArgs[nTailArgs] = NULL;

        int t_pd[2];
        pipe(t_pd);

        int pid = fork();
        if (pid) // Parent
        {
            printf("Process %d forked child %d\n", getpid(), pid);

            close(t_pd[1]);
            dup2(t_pd[0], 0);
            close(t_pd[0]);

            int status;
            pid = wait(&status);
            LOG("ZOMBIE child=%d exitStatus=%x", pid, status);

            exec_command(tailCmd, tailArgs, nTailArgs, envp);
        }
        else // Child
        {
            exec_pipe(headCmd, headArgs, nHeadArgs, envp, t_pd);
        }
    }
    else
    {
        exec_command(cmd, args, nArgs, envp);
    }
}

void exec_command(char* cmd, char* args[], int nArgs, char* envp[])
{
    LOG("Executing command %s:", cmd);
    for (int i = 0; i < nArgs; i++)  
        LOG("arg[%d] = %s", i, args[i]);
    
    // IO Redirect
    for (int i = 0; i < nArgs; i++)
    {
        if (strcmp(args[i], "<")  == 0)
        {
            close(0);
            open(args[i + 1], O_RDONLY);
            args[i] = NULL;
            break;
        }
        else if (strcmp(args[i], ">")  == 0)
        {
            close(1);
            open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            args[i] = NULL;
            break;
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            close(1);
            open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            args[i] = NULL;
            break;
        }
    }

    // Find command path
    char pathName[128] = "\0";
    int found = 0;
    if (is_path(cmd) && access(cmd, X_OK) == 0)
    {
        found = 1;
        strcpy(pathName, cmd);
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
                        strcpy(pathName, dirName);
                        strcat(pathName, "/");
                        strcat(pathName, cmd);
                    }
            }
        }
    }

    // Execute command
    if (found)
    {
        execve(pathName, args, envp);
        {
            LOG("execve failed: errno = %d", errno);
            exit(errno);
        }
    }
    else
    {
        printf("Command %s not found\n", cmd);
        exit(1);
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
