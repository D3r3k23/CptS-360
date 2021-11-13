#include "mkdir_creat.h"

#include "log.h"
#include "global.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

void cmd_mkdir(char* pathname)
{
    if (strlen(pathname) == 0)
    {
        printf("Enter a directory to create");
        return;
    }
    char temp[128];
    char dName[128];
    char bName[128];

    strcpy(temp, pathname);
    strcpy(dName, dirname(temp));

    strcpy(temp, pathname);
    strcpy(bName, basename(temp));

    u32 pino = getino(dName);
    if (pino == 0) // Check if parent inode exists
    {
        printf("Error: %s does not exist", dName);
        return;
    }
    MINODE* pmip = iget(pino);
    if (!S_ISDIR(pmip->INODE.i_mode)) // Check if parent is dir
    {
        printf("Error: %s is not a directory", dName);
        return;
    }
    
    // Check if bName does not exist in dName dir
    if (search(pmip, bName) != 0)
    {
        printf("Error: %s already exists", pathname);
        return;
    }

    mkdir_impl(pmip, bName);
    pmip->INODE.i_links_count++;
    pmip->INODE.i_atime = time(NULL);
    pmip->dirty = 1;
    iput(pmip);
}

void mkdir_impl(MINODE* pmip, char* name)
{
    // Allocate inode and block
    u32 ino = ialloc();
    int blk = balloc();

    // Initialize as dir
    MINODE* mip = iget(ino);
    INODE* ip = &mip->INODE;

    ip->i_mode = 040755;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2; // . & ..
    ip->i_atime = time(NULL);
    ip->i_ctime = time(NULL);
    ip->i_mtime = time(NULL);
    ip->i_blocks = 2;

    ip->i_block[0] = blk;
    for (int i = 1; i < 15; i++)
        ip->i_block[i] = 0;

    mip->dirty = 1;
    iput(mip);

    // Add . & .. entries
    char buf[BLKSIZE];
    get_block(blk, buf);
    DIR* dp = (DIR*)buf;

    // .
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';

    // ..
    dp->inode = pmip->ino;
    dp->rec_len = BLKSIZE - 12;
    dp->name_len = 2;
    dp->name[0] = '.';
    dp->name[1] = '.';

    put_block(blk, buf);

    enter_name(pmip, ino, name);

    pmip->INODE.i_atime = time(NULL);
    pmip->dirty = 1;
    iput(pmip);
}

void cmd_creat(char* pathname)
{
    if (strlen(pathname) == 0)
    {
        printf("Enter a file to create");
        return;
    }
    char temp[128];
    char dName[128];
    char bName[128];

    strcpy(temp, pathname);
    strcpy(dName, dirname(temp));

    strcpy(temp, pathname);
    strcpy(bName, basename(temp));

    u32 pino = getino(dName);
    if (pino == 0) // Check if parent inode exists
    {
        printf("Error: %s does not exist", dName);
        return;
    }
    MINODE* pmip = iget(pino);
    if (!S_ISDIR(pmip->INODE.i_mode)) // Check if parent is dir
    {
        printf("Error: %s is not a directory", dName);
        return;
    }
    
    // Check if bName does not exist in dName dir
    if (search(pmip, bName) != 0)
    {
        printf("Error: %s already exists", pathname);
        return;
    }

    creat_impl(pmip, bName);
    pmip->INODE.i_atime = time(NULL);
    pmip->dirty = 1;
    iput(pmip);
}

void creat_impl(MINODE* pmip, char* name)
{
    // Allocate inode and block
    u32 ino = ialloc();
    int blk = balloc();

    // Initialize as file
    MINODE* mip = iget(ino);
    INODE* ip = &mip->INODE;

    ip->i_mode = 0100644;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 1;
    ip->i_atime = time(NULL);
    ip->i_ctime = time(NULL);
    ip->i_mtime = time(NULL);
    ip->i_blocks = 2;

    ip->i_block[0] = blk;
    for (int i = 1; i < 15; i++)
        ip->i_block[i] = 0;

    mip->dirty = 1;
    iput(mip);

    enter_name(pmip, ino, name);

    pmip->INODE.i_atime = time(NULL);
    pmip->dirty = 1;
    iput(pmip);
}

void enter_name(MINODE* pmip, u32 ino, char* name)
{
    int name_ideal_length = 4 * ((8 + strlen(name) + 3) / 4);

    for (int i = 0; i < 12; i++)
    {
        int blk = pmip->INODE.i_block[i];
        if (blk == 0)
            break;

        LOG("Find last entry in block[%d]", i);
        char buf[BLKSIZE];
        char* cp = get_block(blk, buf);
        DIR* dp = (DIR*)cp;
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            LOG("Entry: %s", dp->name);
            cp += dp->rec_len;
            dp = (DIR*)cp;            
        }

        int ideal_length = 4 * ((8 - dp->name_len + 3) / 4);
        int remaining = dp->rec_len - ideal_length;

        if (name_ideal_length < remaining)
        {
            dp->rec_len = ideal_length;
            cp += dp->rec_len;
            dp = (DIR*)cp;

            dp->inode = ino;
            strcpy(dp->name, name);
            dp->name_len = strlen(name);
            dp->rec_len = remaining;

            put_block(blk, buf);
            return;
        }
        else
        {
            LOG("Not enough space for name %s", name);
            return;
        }
    }
}
