#include "open_close.h"

#include "log.h"
#include "global.h"
#include "util.h"
#include "mkdir_creat.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libgen.h>
#include <sys/stat.h>

int my_open(char* filename, F_MODE mode)
{
    u32 ino = getino(filename);
    if (!ino)
    {
        LOG("Error: File %s does not exist", filename);

        char temp[128];
        char dName[128];
        char bName[128];

        strcpy(temp, filename);
        strcpy(dName, dirname(temp));

        strcpy(temp, filename);
        strcpy(bName, basename(temp));

        u32 pino = getino(dName);
        if (!pino)
        {
            LOG("Error: Parent %s does not exist", dName);
            return -1;
        }
        MINODE* pmip = iget(pino);
        if (!S_ISDIR(pmip->INODE.i_mode))
        {
            LOG("Error: Parent %s is not a directory", dName);
            return -1;
        }

        creat_impl(pmip, bName);
        ino = getino(filename);
    }
    MINODE* mip = iget(ino);

    if (!S_ISREG(mip->INODE.i_mode))
    {
        LOG("Error: %s is not a regular file", filename);
        return -1;
    }

    int lowest_free_fd = -1;
    for (int i = 0; i < NFD; i++)
        if (!running->fd[i])
        {
            lowest_free_fd = i;
            break;
        }
    
    if (lowest_free_fd == -1)
    {
        LOG("No free FD found");
        return -1;
    }

    for (int i = 0; i < lowest_free_fd; i++)
    {
        OFT* oft = running->fd[i];
        F_MODE open_mode = oft->mode;
        if (oft->mip == mip && !(open_mode == RD && mode == RD))
        {
            LOG("Error: %s is already open with mode: %d", filename, open_mode);
            return -1;
        }
    }
    if (mode == WR)
        truncate(mip);

    OFT* new_oft = make_oft(mip, mode);
    running->fd[lowest_free_fd] = new_oft;

    mip->INODE.i_atime = time(NULL);
    if (mode != RD)
        mip->INODE.i_mtime = time(NULL);
    
    mip->dirty = 1;
    iput(mip);

    return lowest_free_fd;
}

void truncate(MINODE* mip)
{
    INODE* ip = &mip->INODE;

    // 12 direct blocks
    for (int i = 0; i < 12; i++)
        if (ip->i_block[i])
        {
            bdalloc(ip->i_block[i]);
            ip->i_block[i] = 0;
        }

    int nIndirect_blocks = BLKSIZE / sizeof(u32);;
    
    // 256 indirect blocks
    if (ip->i_block[12])
    {
        char buf[BLKSIZE];
        u32* indirect_blk = (u32*)get_block(ip->i_block[12], buf);

        for (int i = 0; i < nIndirect_blocks; i++)
        {
            if (*indirect_blk)
            {
                bdalloc(*indirect_blk);
                *indirect_blk = 0;
            }
            indirect_blk++;
        }

        bdalloc(ip->i_block[12]);
        ip->i_block[12] = 0;
    }

    // 256*256 double indirect blocks
    if (ip->i_block[13])
    {
        char buf1[BLKSIZE];
        u32* dbl_indirect_blk = (u32*)get_block(ip->i_block[13], buf1);

        for (int i = 0; i < nIndirect_blocks; i++)
        {
            if (*dbl_indirect_blk)
            {
                char buf2[BLKSIZE];
                u32* sin_indirect_blk = (u32*)get_block(*dbl_indirect_blk, buf2);

                for (int j = 0; j < nIndirect_blocks; j++)
                {
                    if (*sin_indirect_blk)
                    {
                        bdalloc(*sin_indirect_blk);
                        *sin_indirect_blk = 0;
                    }
                    sin_indirect_blk++;
                }
                bdalloc(*dbl_indirect_blk);
                *dbl_indirect_blk = 0;
            }
            dbl_indirect_blk++;
        }
        bdalloc(ip->i_block[13]);
        ip->i_block[13] = 0;
    }

    ip->i_blocks = 0;
    ip->i_size = 0;

    ip->i_atime = time(NULL);
    ip->i_ctime = time(NULL);
    ip->i_mtime = time(NULL);

    mip->dirty = 1;
    iput(mip);
}

OFT* make_oft(MINODE* mip, F_MODE mode)
{
    OFT* oft = (OFT*)malloc(sizeof(OFT));

    oft->mode = mode;
    oft->refCount = 1;
    oft->mip = mip;
    
    if (mode == AP)
        oft->offset = mip->INODE.i_size;
    else
        oft->offset = 0;
    
    return oft;
}

int my_close(int fd)
{
    if (fd < 0 || NFD - 1 < fd)
    {
        LOG("Error: FD %d out of range", fd);
        return -1;
    }

    OFT* oft = running->fd[fd];
    if (!oft)
    {
        LOG("Error: No OFT exists for FD %D", fd);
        return -1;
    }
    running->fd[fd] = NULL;

    oft->refCount--;
    if (oft->refCount <= 0)
    {
        oft->mip->dirty = 1;
        iput(oft->mip);

        free(oft);
    }
    return 0;
}

int lseek(int fd, u32 position)
{
    if (fd < 0 || NFD - 1 < fd)
    {
        LOG("Error: FD %d out of range", fd);
        return -1;
    }

    OFT* oft = running->fd[fd];
    if (!oft)
    {
        LOG("Error: No OFT exists for FD %D", fd);
        return -1;
    }

    if (position > oft->mip->INODE.i_size)
    {
        LOG("Error: file size overrun");
        return -1;
    }

    int original_position = oft->offset;
    oft->offset = position;
    return original_position;
}

void cmd_pfd(void)
{
    printf(" fd  mode offset  INODE \n");
    printf("---- ---- ------ -------\n");
    for (int i = 0; i < NFD; i++)
    {
        OFT* oft = running->fd[i];
        if (!oft)
            return;
        else
        {
            MINODE* mip = oft->mip;
            printf(" %2d    %d    %4d  [%d, %d]\n", i, oft->mode, oft->offset, mip->dev, mip->ino);
        }
    }
}
