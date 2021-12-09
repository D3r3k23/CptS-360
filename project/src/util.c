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

int tst_bit(char* buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8));
}

void set_bit(char* buf, int bit)
{
    buf[bit / 8] |= (1 << (bit % 8));
}

void clr_bit(char* buf, int bit)
{
    buf[bit / 8] &= ~(1 << (bit % 8));
}

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
            // LOG("found [%d %d] as minode[%d] in core", dev, ino, i);
            return mip;
        }
    }
    /////// TEST
    for (int i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount == 0)
        {
            // LOG("allocating NEW minode[%d] for [%d %d]", i, dev, ino);
            mip->refCount = 1;
            mip->dev = dev;
            mip->ino = ino;

            // get INODE of ino to buf    
            blk    = (ino-1)/8 + iblk;
            offset = (ino-1) % 8;

            // LOG("iget: ino=%d blk=%d offset=%d", ino, blk, offset);

            get_block(blk, buf);
            ip = (INODE*)buf + offset;
            // copy INODE to mp->INODE
            mip->INODE = *ip;
            return mip;
        }
    }   
    LOG("PANIC: no more free minodes");
    return 0;
}

void iput(MINODE *mip)
{
    if (!mip)
        return;

    mip->refCount--;

    if (mip->refCount > 0)
        return;
    if (!mip->dirty)
        return;

    /* write INODE back to disk */
    /**************** NOTE ******************************
     For mountroot, we never MODIFY any loaded INODE
                    so no need to write it back
    FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

    Write YOUR code here to write INODE back to disk
    *****************************************************/

   int block  = (mip->ino - 1) / 8 + iblk;
   int offset = (mip->ino - 1) % 8;

   char buf[BLKSIZE];
   get_block(block, buf);

   INODE* ip = (INODE*)buf + offset;
   *ip = mip->INODE;

   put_block(block, buf);
} 

u32 search(MINODE *mip, char *name)
{
    char *cp, sbuf[BLKSIZE], temp[256];
    DIR *dp;
    INODE *ip;

    LOG("search for %s in MINODE = [%d, %d]", name, mip->dev, mip->ino);
    ip = &(mip->INODE);

    /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

    get_block(ip->i_block[0], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    LOG("  ino   rlen  nlen  name");

    while (cp < sbuf + BLKSIZE)
    {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        LOG("%4d  %4d  %4d    %s", dp->inode, dp->rec_len, dp->name_len, dp->name);
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

    LOG("pathname=%s", pathname);
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
        LOG("i=%d name[%d]=%s", i, i, name[i]);

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

    if (my_ino)
        *my_ino = dp->inode;

    cp += dp->rec_len;
    dp = (DIR*)cp;
    return dp->inode;
}

void inc_free_inodes(void)
{
    char buf[BLKSIZE];

    get_block(SUPER_INO, buf);
    SUPER* sp = (SUPER*)buf;
    sp->s_free_inodes_count++;
    put_block(SUPER_INO, buf);

    get_block(ROOT_INO, buf);
    GD* gp = (GD*)buf;
    gp->bg_free_inodes_count++;
    put_block(ROOT_INO, buf);
}

void dec_free_inodes(void)
{
    char buf[BLKSIZE];

    get_block(SUPER_INO, buf);
    SUPER* sp = (SUPER*)buf;
    sp->s_free_inodes_count--;
    put_block(SUPER_INO, buf);

    get_block(ROOT_INO, buf);
    GD* gp = (GD*)buf;
    gp->bg_free_inodes_count--;
    put_block(ROOT_INO, buf);
}

void inc_free_blocks(void)
{
    char buf[BLKSIZE];

    get_block(SUPER_INO, buf);
    SUPER* sp = (SUPER*)buf;
    sp->s_free_blocks_count++;
    put_block(SUPER_INO, buf);

    get_block(ROOT_INO, buf);
    GD* gp = (GD*)buf;
    gp->bg_free_blocks_count++;
    put_block(ROOT_INO, buf);
}

void dec_free_blocks(void)
{
    char buf[BLKSIZE];

    get_block(SUPER_INO, buf);
    SUPER* sp = (SUPER*)buf;
    sp->s_free_blocks_count--;
    put_block(SUPER_INO, buf);

    get_block(ROOT_INO, buf);
    GD* gp = (GD*)buf;
    gp->bg_free_blocks_count--;
    put_block(ROOT_INO, buf);
}

int ialloc(void)
{
    char buf[BLKSIZE];
    get_block(imap, buf);

    for (int i = 0; i < ninodes; i++)
    {
        if (!tst_bit(buf, i))
        {
            set_bit(buf, i);
            put_block(imap, buf);

            dec_free_inodes();
            u32 ino = i + 1;
            LOG("Allocated inode %d", ino);
            return ino;
        }
    }
    LOG("Error: No free inodes");
    return 0;
}

int balloc(void)
{
    char buf[BLKSIZE];
    get_block(bmap, buf);

    for (int i = 0; i < nblocks; i++)
    {
        if (!tst_bit(buf, i))
        {
            set_bit(buf, i);
            put_block(bmap, buf);

            dec_free_blocks();
            u32 blk = i + 1;
            LOG("Allocated block %d", blk);
            return blk;
        }
    }
    LOG("Error: No free blocks");
    return 0;
}

void midalloc(MINODE* mip)
{
    mip->refCount = 0;
}

void idalloc(u32 ino)
{
    if (ino > ninodes)
    {
        LOG("Inode %d out of range", ino);
        return;
    }

    char buf[BLKSIZE];
    get_block(imap, buf);

    clr_bit(buf, ino - 1);
    put_block(imap, buf);

    inc_free_inodes();
}

void bdalloc(int blk)
{
    if (blk > nblocks)
    {
        LOG("Block %d out of range", blk);
        return;
    }

    char buf[BLKSIZE];
    get_block(bmap, buf);

    clr_bit(buf, blk - 1);
    put_block(bmap, buf);

    inc_free_blocks();
}

int tokenize(char *pathname)
{
    LOG("pathname: %s", pathname);

    strcpy(gpath, pathname); // tokens are in global gpath[ ]
    int n = 0;
    char* s = strtok(gpath, "/");
    while (s)
    {
        name[n] = s;
        n++;
        s = strtok(NULL, "/");
    }
    name[n] = '\0';

    for (int i = 0; i < n; i++)
        LOG("%s", name[i]);

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

int is_empty(MINODE* mip)
{
    INODE* ip = &mip->INODE;

    if (ip->i_links_count > 2)
        return 0;

    else if (ip->i_links_count == 2)
        for (int i = 0; i < 12; i++)
            if (ip->i_block[i])
            {
                char buf[BLKSIZE];
                char* cp = get_block(ip->i_block[i], buf);

                while (cp < buf + BLKSIZE)
                {
                    DIR* dp = (DIR*)cp;

                    char name[256];
                    strncpy(name, dp->name, dp->name_len);
                    name[dp->name_len] = '\0';

                    if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
                        return 0;
                    
                    cp += dp->rec_len;
                }
            }

    return 1;
}

u32 map(INODE* ip, u32 log_blk, int do_balloc)
{
    if (log_blk < 12) // Direct block
    {
        if (!ip->i_block[log_blk] && do_balloc)
        {
            ip->i_blocks++;
            ip->i_block[log_blk] = balloc();
        }
        return ip->i_block[log_blk];
    }
    else if (12 <= log_blk && log_blk < 12 + 256) // Indirect blocks
    {
        if (!ip->i_block[12] && do_balloc)
        {
            ip->i_blocks++;
            ip->i_block[12] = balloc();
            char zero[BLKSIZE];
            put_block(ip->i_block[12], zero);
        }
        char buf[BLKSIZE];
        u32* indirect_blk = (u32*)get_block(ip->i_block[12], buf);

        if (!indirect_blk[log_blk - 12] && do_balloc)
        {
            ip->i_blocks++;
            indirect_blk[log_blk - 12] = balloc();
        }
        return indirect_blk[log_blk - 12];
    }
    else // Double indirect blocks
    {
        if (!ip->i_block[13] && do_balloc)
        {
            ip->i_blocks++;
            ip->i_block[13] = balloc();
            char zero[BLKSIZE];
            put_block(ip->i_block[13], zero);
        }
        char buf1[BLKSIZE];
        u32* dbl_indirect_blk = (u32*)get_block(ip->i_block[13], buf1);
        u32 n_dbl_indirect_blks = BLKSIZE / sizeof(u32);

        u32 log_indirect_blk = log_blk - n_dbl_indirect_blks - 12;
        u32 indirect_blk = dbl_indirect_blk[log_indirect_blk / n_dbl_indirect_blks];
        if (!indirect_blk && do_balloc)
        {
            ip->i_blocks++;
            indirect_blk = dbl_indirect_blk[log_indirect_blk / n_dbl_indirect_blks] = balloc();
            char zero[BLKSIZE];
            put_block(indirect_blk, zero);
        }

        char buf2[BLKSIZE];
        u32* sin_indirect_blk = (u32*)get_block(indirect_blk, buf2);

        if (!sin_indirect_blk[log_indirect_blk % n_dbl_indirect_blks] && do_balloc)
        {
            ip->i_blocks++;
            sin_indirect_blk[log_indirect_blk % n_dbl_indirect_blks] = balloc();
        }
        return sin_indirect_blk[log_indirect_blk % n_dbl_indirect_blks];
    }
}
