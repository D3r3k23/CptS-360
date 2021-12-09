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
    
    INODE* ip = &oft->mip->INODE;

    size_t n = 0;
    while (count)
    {
        const u32 log_blk = oft->offset / BLKSIZE;
        const u32 blk = map(ip, log_blk, 1);
        const size_t start = offset % BLKSIZE;

        char write_buf[BLKSIZE];
        char* cp = get_block(blk, write_buf) + start;

        const size_t remainder = BLKSIZE - start;
        const size_t nBytes = min(count, remainder);

        LOG("offset=%d log_blk=%u blk=%u: Copying %u bytes", offset, log_blk, blk, nBytes);
        memcpy(cp, in_buf, nBytes);
        put_block(blk, write_buf);

        in_buf += nBytes;
        n += nBytes;
        offset += nBytes;
        ip->i_size += nBytes;

        count -= nBytes;
    }
    oft->offset = offset;

    LOG("Wrote %u bytes from FD %d", n, fd);
    return n;
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
