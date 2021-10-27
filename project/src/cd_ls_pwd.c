#include "cd_ls_pwd.h"

#include "global.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

void cd()
{
    printf("cd: under construction READ textbook!!!!\n");

    // READ Chapter 11.7.3 HOW TO chdir
}

void ls_file(MINODE *mip, char *name)
{
    printf("ls_file: to be done: READ textbook!!!!\n");
    // READ Chapter 11.7.3 HOW TO ls
}

void ls_dir(MINODE *mip)
{
    printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

    char buf[BLKSIZE], temp[256];
    DIR *dp;
    char *cp;

    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;
    
    while (cp < buf + BLKSIZE)
    {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        
        printf("%s  ", temp);

        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
    printf("\n");
}

void ls()
{
    printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
    ls_dir(running->cwd);
}

void pwd(MINODE *wd)
{
    printf("pwd: READ HOW TO pwd in textbook!!!!\n");
    if (wd == root)
    {
        printf("/\n");
        return;
    }
}
