#include "util.h"

#include "log.h"
#include "global.h"

#include <ext2fs/ext2_fs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

char* get_block(int blk, char* buf)
{
    lseek(dev, (long)blk*BLKSIZE, 0);
    read(dev, buf, BLKSIZE);
    return buf;
}

int put_block(int blk, char* buf)
{
    lseek(dev, (long)blk*BLKSIZE, 0);
    return write(dev, buf, BLKSIZE);
}

// return minode pointer to loaded INODE
MINODE *iget(u32 ino)
{
    MINODE *mip;
    char buf[BLKSIZE];
    int blk, offset;
    INODE *ip;

    for (int i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount && mip->dev == dev && mip->ino == ino)
        {
            mip->refCount++;
            //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
            return mip;
        }
    }

    for (int i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount == 0)
        {
            //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
            mip->refCount = 1;
            mip->dev = dev;
            mip->ino = ino;

            // get INODE of ino to buf    
            blk    = (ino-1)/8 + iblk;
            offset = (ino-1) % 8;

            //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

            get_block(blk, buf);
            ip = (INODE*)buf + offset;
            // copy INODE to mp->INODE
            mip->INODE = *ip;
            return mip;
        }
    }   
    printf("PANIC: no more free minodes\n");
    return 0;
}

void iput(MINODE *mip)
{
    if (!mip)
        return;

    int block, offset;
    char buf[BLKSIZE];
    INODE *ip;

    mip->refCount--;

    if (mip->refCount > 0) return;
    if (!mip->dirty)       return;

    /* write INODE back to disk */
    /**************** NOTE ******************************
     For mountroot, we never MODIFY any loaded INODE
                    so no need to write it back
    FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

    Write YOUR code here to write INODE back to disk
    *****************************************************/
} 

u32 search(MINODE *mip, char *name)
{
    char *cp, sbuf[BLKSIZE], temp[256];
    DIR *dp;
    INODE *ip;

    printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
    ip = &(mip->INODE);

    /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

    get_block(ip->i_block[0], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    printf("  ino   rlen  nlen  name\n");

    while (cp < sbuf + BLKSIZE)
    {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        printf("%4d  %4d  %4d    %s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
        if (strcmp(temp, name) == 0)
        {
            LOG("found %s : ino = %d", temp, dp->inode);
            return dp->inode;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    return 0;
}

u32 getino(char *pathname)
{
    MINODE *mip;

    printf("getino: pathname=%s\n", pathname);
    if (strcmp(pathname, "/") == 0)
        return ROOT_INO;

    // starting mip = root OR CWD
    if (pathname[0] == '/')
        mip = root;
    else
        mip = running->cwd;

    mip->refCount++; // because we iput(mip) later

    int n = tokenize(pathname);
    int ino;
    for (int i = 0; i < n; i++)
    {
        printf("===========================================\n");
        printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);

        ino = search(mip, name[i]);

        if (ino == 0)
        {
            iput(mip);
            LOG("name %s does not exist", name[i]);
            return 0;
        }
        iput(mip);
        mip = iget(ino);
    }

    iput(mip);
    return ino;
}

// my_name[] should be size 256
int findmyname(MINODE *parent, u32 my_ino, char my_name[]) 
{
    char buf[BLKSIZE];
    char* cp = get_block(parent->INODE.i_block[0], buf);
    while (cp < buf + BLKSIZE)
    {
        DIR* dp = (DIR*)cp;
        if (dp->inode == my_ino)
        {
            int nMax = min(dp->name_len, 255);
            strncpy(my_name, dp->name, nMax + 1);
            my_name[nMax] = '\0';
            return 1; // Found
        }
        cp += dp->rec_len;
    }
    return 0; // Not found
}

// myino = i# of . | return i# of ..
// mip points at a DIR minode
u32 findino(MINODE* mip, u32* my_ino)
{
    char buf[BLKSIZE];
    char* cp = get_block(mip->INODE.i_block[0], buf);
    DIR* dp = (DIR*)buf;
    *my_ino = dp->inode;

    cp += dp->rec_len;
    dp = (DIR*)cp;
    return dp->inode;
}

int tokenize(char *pathname)
{
    printf("tokenize %s\n", pathname);

    strcpy(gpath, pathname); // tokens are in global gpath[ ]
    int n = 0;
    char* s = strtok(gpath, "/");
    while (s)
    {
        name[n] = s;
        n++;
        s = strtok(NULL, "/");
    }
    name[n] = 0;

    for (int i = 0; i < n; i++)
        printf("%s  ", name[i]);
    printf("\n");
    return n;
}

int streq(const char* s1, const char* s2)
{
    return strcmp(s1, s2) == 0;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}
