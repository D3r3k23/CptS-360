#include "global.h"
#include "log.h"
#include "type.h"
#include "util.h"
#include "cd_ls_pwd.h"

#include <ext2fs/ext2_fs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>

void init();
void mount_root();
void quit();

const char *disk = "diskimage";

int main(int argc, char* argv[])
{
    int ino;
    char buf[BLKSIZE];

    printf("checking EXT2 FS ...");
    if ((fd = open(disk, O_RDWR)) < 0)
    {
        printf("open %s failed\n", disk);
        exit(1);
    }

    dev = fd;    // global dev same as this fd   

    /********** read super block  ****************/
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        exit(1);
    }     
    printf("EXT2 FS OK\n");
    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;

    get_block(dev, 2, buf); 
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    iblk = gp->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

    init();  
    mount_root();
    printf("root refCount = %d\n", root->refCount);

    printf("creating P0 as running process\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = iget(dev, 2);
    printf("root refCount = %d\n", root->refCount);

    // WRTIE code here to create P1 as a USER process

    while(1)
    {
        printf("input command : [ls|cd|pwd|quit] ");
        fgets(line, 128, stdin);
        line[strlen(line)-1] = 0;

        if (line[0] == 0)
            continue;
        pathname[0] = 0;

        sscanf(line, "%s %s", cmd, pathname);
        printf("cmd=%s pathname=%s\n", cmd, pathname);
    
        if (strcmp(cmd, "ls") == 0)
            ls(pathname);
        else if (strcmp(cmd, "cd") == 0)
            chdir(pathname);
        else if (strcmp(cmd, "pwd") == 0)
            pwd(running->cwd);
        else if (strcmp(cmd, "quit") == 0)
            quit();
    }
}

void init()
{
    LOG("init");
    for (int i = 0; i < NMINODE; i++)
    {
        minode[i].dev = 0;
        minode[i].ino = 0;
        minode[i].refCount = 0;
        minode[i].mounted = 0;
        minode[i].mptr = 0;
    }
    for (int i = 0; i < NPROC; i++)
    {
        proc[i].pid = i;
        proc[i].uid = 0;
        proc[i].gid = 0;
        proc[i].cwd = 0;
    }
}

// load root INODE and set root pointer to it
void mount_root()
{  
    LOG("mount_root");
    root = iget(dev, 2);
}

void quit()
{
    int i;
    MINODE *mip;
    for (i=0; i<NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount > 0)
            iput(mip);
    }
    exit(0);
}

int chdir(char *pathname)
{
    INODE * current;
    if (pathname[0] == '/') //validate its a dir
    {
       current = root;
       pathname++;
	}
    else{
        current = cwd;
	}
    char * tok = strtok(path, "/"); //tokenize path
    do
    {
     int num = search(current,tok);
     if (num == 0)
     {
      return 0;
	 }
     current = getino(dev); //from global.h
	} while (token = strtok(NULL,"/")); //continue until reach end
    cwd = current;
    return 1;//success
}

