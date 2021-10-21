#include "commands.h"

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
    "menu",
    "get",
    "put",
    "ls",
    "cd",
    "pwd",
    "mkdir",
    "rmdir",
    "rm",
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

void c_cat(const char* pathname, FILE* f)
{
    FILE* file = fopen(pathname, "r");
    if (!file)
        fprintf(f, "Unable to open file %s\n", pathname);
    else
    {
        char line[256];
        while (fgets(line, 256, file))
            fprintf(f, "%s", line);
    }
}

void c_ls(const char* pathname, FILE* f)
{
    char cwd[128];
    if (!pathname || strlen(pathname) == 0)
        pathname = getcwd(cwd, 128);

    struct stat st;
    int r = lstat(pathname, &st);
    if (r == -1)
    {
        fprintf(f, "Unable to stat %s\n", pathname);
        return;
    }

    if (S_ISREG(st.st_mode))
        ls_file(pathname, f);
    else if (S_ISDIR(st.st_mode))
        ls_dir(pathname, f);
}

void c_cd(const char* pathname)
{
    chdir(pathname);
}

void c_pwd(FILE* f)
{
    char buf[128];
    fprintf(f, "%s\n", getcwd(buf, 128));
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

void ls_dir(const char* pathname, FILE* f)
{
    DIR* dir = opendir(pathname);
    if (!dir)
        fprintf(f, "Unable to open dir %s\n", pathname);
    else
    {
        struct dirent* ent;
        while (ent = readdir(dir))
            if (ent->d_name[0] != '.')
                ls_file(ent->d_name, f);
    }
    closedir(dir);
}

void ls_file(const char* pathname, FILE* f)
{
    static const char* modes = "rwxrwxrwx";

    struct stat st;
    int r = lstat(pathname, &st);
    if (r == -1)
    {
        fprintf(f, "Unable to stat %s\n", pathname);
        return;
    }

    if (S_ISREG(st.st_mode))
        fprintf(f, "%c", '-');
    else if (S_ISDIR(st.st_mode))
        fprintf(f, "%c", 'd');
    else if (S_ISLNK(st.st_mode))
        fprintf(f, "%c", 'l');

    for (int i = 0; i < 9; i++)
        if (st.st_mode & (1 << (8 - i)))
            fprintf(f, "%c", modes[i]);
        else
            fprintf(f, "%c", '-');
        
    fprintf(f, "%3lu ", st.st_nlink);
    fprintf(f, "%4d ",  st.st_gid);
    fprintf(f, "%4d ",  st.st_uid);
    fprintf(f, "%5ld ", st.st_size);

    fprintf(f, "%s ", ctime(&st.st_ctime));

    char tmp[128];
    strcpy(tmp, pathname);
    fprintf(f, "%s", basename(tmp));

    if (S_ISLNK(st.st_mode))
    {
        char linkname[128];
        readlink(pathname, linkname, 128);
        linkname[127] = '\0';
        fprintf(f, " -> %s", linkname);
    }

    fprintf(f, "\n");
}
