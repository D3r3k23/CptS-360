//test
#include "global.h"
#include "log.h"
#include "type.h"
#include "util.h"

// Commands
#include "cd_ls_pwd.h"
#include "mkdir_creat.h"
#include "rmdir.h"
#include "link_unlink.h"
#include "symlink.h"

#include <ext2fs/ext2_fs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>

//------ Globals ------//

MINODE minode[NMINODE];
MINODE* root;
PROC proc[NPROC];
PROC* running;

char gpath[128]; // global for tokenized components
char* name[64];  // assume at most 64 components in pathname

int dev=0;
int nblocks=0, ninodes=0, bmap, imap=0, iblk=0;

//---------------------//

void init();
void mount_root();
void cmd_quit();

const char* disk = "diskimage";

int main(int argc, char* argv[])
{
    init();  
    mount_root();
    printf("root refCount = %d\n", root->refCount);

    printf("creating P0 as running process\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = iget(ROOT_INO);
    printf("root refCount = %d\n", root->refCount);

    // WRTIE code here to create P1 as a USER process

    


    char line[256], cmd[64], pathname1[128], pathname2[128];
    
    while (1)
    {
        memset(line, 0, 256);
        memset(cmd, 0, 64);
        memset(pathname1, 0, 128);
        memset(pathname2, 0, 128);

        printf("[ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink|quit]\n");
        printf("Input command: ");
        fgets(line, 128, stdin);

        sscanf(line, "%s %s %s", cmd, pathname1, pathname2);
        LOG("cmd=%s pathname1=%s pathname2=%s", cmd, pathname1, pathname2);
    
        if      (streq(cmd, "ls"))       cmd_ls(pathname1);
        else if (streq(cmd, "cd"))       cmd_cd(pathname1);
        else if (streq(cmd, "pwd"))      cmd_pwd();
        else if (streq(cmd, "mkdir"))    cmd_mkdir(pathname1);
        else if (streq(cmd, "creat"))    cmd_creat(pathname1);
        else if (streq(cmd, "rmdir"))    cmd_rmdir(pathname1);
        else if (streq(cmd, "link"))     cmd_link(pathname1, pathname2);
        else if (streq(cmd, "unlink"))   cmd_unlink(pathname1);
        else if (streq(cmd, "symlink"))  cmd_symlink(pathname1, pathname2);
        else if (streq(cmd, "readlink")) cmd_readlink(pathname1);
        else if (streq(cmd, "quit"))     cmd_quit();
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

void cmd_quit()
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
