#include "write_cp.h"

#include "log.h"
#include "global.h"
#include "util.h"
#include "open_close.h"

#include <stdio.h>
#include <string.h>

int my_write(int fd, char* in_buf, size_t count)
{
    OFT* oft = running->fd[fd];
    if (!oft)
    {
        LOG("Error: FD %d is not open", fd);
        return -1;
    }

    F_MODE mode = oft->mode;
    int offset = oft->offset;

    if (mode == RD)
        return -1;
    
    MINODE* mip = oft->mip;
    INODE* ip = &mip->INODE;

    while (count)
    {
        u32 log_blk = oft->offset / BLKSIZE;
        u32 blk = map(ip, log_blk, 1);
        size_t start = offset % BLKSIZE;

        if (ip->i_block[blk]);
    }
}

void cmd_cp(char* src, char* dest)
{
    
}

void cmd_mv(char* src, char* dest)
{
    u32 src_ino  = getino(src);
    u32 dest_ino = getino(dest);

    MINODE* src_mip  = iget(src_ino);
    MINODE* dest_mip = iget(dest_ino);

    if (src_mip->dev == dest_mip->dev)
    {
        
    }
    else
    {
        // Level 3
    }
}
