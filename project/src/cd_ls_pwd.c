#include "cd_ls_pwd.h"

#include "global.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

void cd(char* pathname)
{
    // READ Chapter 11.7.3 HOW TO chdir

    // if (!pathname)
    // {

    // }

    // INODE * current;
    // if (pathname[0] == '/') //validate its a dir
    // {
    //    current = root;
    //    pathname++;
	// }
    // else{
    //     current = cwd;
	// }
    // char * tok = strtok(path, "/"); //tokenize path
    // do
    // {
    //  int num = search(current,tok);
    //  if (num == 0)
    //  {
    //   return;
	//  }
    //  current = getino(dev); //from global.h
	// } while (token = strtok(NULL,"/")); //continue until reach end
    // cwd = current;
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

    get_block(mip->INODE.i_block[0], buf);
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

void pwd()
{
    MINODE *cwd = running->cwd;
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
