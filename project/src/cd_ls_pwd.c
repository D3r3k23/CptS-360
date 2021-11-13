#include "cd_ls_pwd.h"

#include "log.h"
#include "global.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>

void cmd_cd(char* pathname)
{
    int ino = getino(pathname);//returns error if ino = 0 
    MINODE *mip = iget(ino);
    if(S_ISDIR(mip->INODE.i_mode)) //check if dir
    {
        iput(running->cwd);
        running->cwd = mip;
    }
    else
    {
        printf("cd error: not a valid directory");
    }
}

void cmd_ls(char* pathname)
{
    MINODE* mip;
    if (strlen(pathname) == 0)
        mip = running->cwd;
    else
    {
        u32 ino = getino(pathname);
        if (ino == 0)
        {
            printf("Error: %s does not exist.\n", pathname);
            return;
        }
        mip = iget(ino);
    }

    if (S_ISDIR(mip->INODE.i_mode))
        ls_dir(mip);
    else
        ls_file(mip, pathname);
}

void ls_dir(MINODE* mip)
{
    char buf[BLKSIZE];
    char* cp = get_block(mip->INODE.i_block[0], buf);
    while (cp < buf + BLKSIZE)
    {
        DIR* dp = (DIR*)cp;

        char name[256];
        int nMax = min(dp->name_len, 255);
        strncpy(name, dp->name, nMax + 1);
        name[nMax] = '\0';
        
        mip = iget(dp->inode);
        ls_file(mip, name);

        cp += dp->rec_len;
    }
}

void ls_file(MINODE* mip, char* name)
{
    static const char* modes = "rwxrwxrwx";

    INODE* ip = &(mip->INODE);

    if (S_ISREG(ip->i_mode))
        printf("%c", '-');
    if (S_ISDIR(ip->i_mode))
        printf("%c", 'd');
    if (S_ISLNK(ip->i_mode))
        printf("%c", 'l');
    
    for (int i = 0; i < 9; i++)
        if (ip->i_mode & (0x1 << (8 - i)))
            printf("%c", modes[i]);
        else
            printf("%c", '-');
    
    printf("%2u ", ip->i_links_count);
    printf("%2u ", ip->i_gid);
    printf("%2u ", ip->i_uid);
    printf("%6u ", ip->i_size);

    char temp[128];

    strcpy(temp, ctime((time_t*)&(ip->i_mtime)));
    temp[strlen(temp)-1] = '\0'; // Trim NL
    printf("%s ", temp);

    strcpy(temp, name);
    printf("%s", basename(temp));

    char* linkname = (char*)(ip->i_block);
    if (S_ISLNK(ip->i_mode))
        printf(" -> %s", linkname);
    
    printf("\n");
}

void cmd_pwd()
{
    MINODE* cwd = running->cwd;
    if (cwd == root)
        printf("/");
    else
        rec_pwd(cwd);
    printf("\n");
}

void rec_pwd(MINODE* wd)
{
    if (wd == root)
        return;

    char buf[BLKSIZE];
    get_block(wd->INODE.i_block[0], buf);

    u32 my_ino;
    u32 parent_ino = findino(wd, &my_ino);
    MINODE* pip = iget(parent_ino);

    char name[256];
    findmyname(pip, my_ino, name);

    rec_pwd(pip);
    printf("/%s", name);
}
