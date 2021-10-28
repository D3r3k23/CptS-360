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

const char* disk = "diskimage";

int main(int argc, char* argv[])
{
    init();  
    mount_root();
    printf("root refCount = %d\n", root->refCount);

    printf("creating P0 as running process\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = iget(2);
    printf("root refCount = %d\n", root->refCount);


    // WRTIE code here to create P1 as a USER process

    

    while (1)
    {
        memset(line, 0, 128);
        memset(cmd, 0, 64);
        memset(pathname, 0, 128);

        printf("input command : [ls|cd|pwd|quit] ");
        fgets(line, 128, stdin);

        sscanf(line, "%s %s", cmd, pathname);
        LOG("cmd=%s pathname=%s", cmd, pathname);
    
        if (streq(cmd, "ls"))
            ls(pathname);
        else if (streq(cmd, "cd"))
            cd(pathname);
        else if (streq(cmd, "pwd"))
            pwd();
        else if (streq(cmd, "quit"))
            quit();
    }
}

void init()
{
    LOG("init");

    char buf[BLKSIZE];

    printf("checking EXT2 FS ...");
    if ((dev = open(disk, O_RDWR)) < 0)
    {
        printf("open %s failed\n", disk);
        exit(1);
    }

    /********** read super block  ****************/
    get_block(1, buf);
    SUPER* sp = (SUPER*)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        exit(1);
    }     
    printf("EXT2 FS OK\n");
    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;

    get_block(ROOT_INO, buf); 
    GD* gp = (GD*)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    iblk = gp->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

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
    root = iget(ROOT_INO);
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
