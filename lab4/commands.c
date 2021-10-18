#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

static const char* COMMANDS[CMD_COUNT] = {
    "exit",
    "get",
    "put",
    "ls",
    "cd",
    "pwd",
    "mkdir",
    "rmdir",
    "rm",
    "lexit",
    "lcat",
    "lls",
    "lcd",
    "lpwd",
    "lmkdir",
    "lrmdir",
    "lrm"
};

CMD find_cmd(const char* cmd)
{
    for (int i = 0; i < CMD_COUNT; i++)
        if (strcmp(cmd, COMMANDS[i]) == 0)
            return (CMD)i;
    return CMD_NONE;
}

void c_cat(const char* pathname)
{
    FILE* file = fopen(pathname, "r");
    if (!file)
        printf("Unable to open file %s\n", pathname);
    else
    {
        char line[256];
        while (fgets(line, 256, file))
        {
            printf("%s\n", line);
        }
    }
}

void c_ls(const char* pathname)
{
    char cwd[128];
    if (!pathname || strlen(pathname) == 0)
        pathname = getcwd(cwd, 128);

    struct stat st;
    int r = lstat(pathname, &st);
    if (r == -1)
    {
        printf("Unable to open %s\n", pathname);
        return;
    }

    if (S_ISREG(st.st_mode))
        ls_file(pathname);
    else if (S_ISDIR(st.st_mode))
        ls_dir(pathname);
}

void c_cd(const char* pathname)
{
    chdir(pathname);
}

void c_pwd(void)
{
    char buf[128];
    printf("%s\n", getcwd(buf, 128));
}

void c_mkdir(const char* pathname)
{
    mkdir(pathname, 0766);
}

void c_rmdir(const char* pathname)
{
    rmdir(pathname);
}

void c_rm(const char* pathname)
{
    unlink(pathname);
}

void ls_dir(const char* pathname)
{
    DIR* dir = opendir(pathname);
    if (!dir)
        printf("Unable to open dir %s\n", pathname);
    else
    {
        struct dirent* ent;
        while (ent = readdir(dir))
        {
            if (ent->d_name[0] == '.')
                continue;
            else
                ls_file(ent->d_name);
        }
    }
    closedir(dir);
}

void ls_file(const char* pathname)
{
    static const char* modes = "rwxrwxrwx";

    struct stat st;
    lstat(pathname, &st);

    if (S_ISREG(st.st_mode))
        printf("%c", '-');
    else if (S_ISDIR(st.st_mode))
        printf("%c", 'd');
    else if (S_ISLNK(st.st_mode))
        printf("%c", 'l');

    for (int i = 0; i < 9; i++)
        if (st.st_mode & (1 << (8 - i)))
            printf("%c", modes[i]);
        else
            printf("%c", '-');
        
    printf("%4d ", st.st_nlink);
    printf("%4d ", st.st_gid);
    printf("%4d ", st.st_uid);
    printf("%4d ", st.st_size);

    printf("%s ", ctime(&st.st_ctime));

    char tmp[128];
    strcpy(tmp, pathname);
    printf("%s", basename(tmp));

    if (S_ISLNK(st.st_mode))
    {
        char linkname[128];
        readlink(pathname, linkname, 128);
        linkname[127] = '\0';
        printf(" -> %s", linkname);
    }

    printf("\n");
}
