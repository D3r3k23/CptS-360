#include "read_cat.h"

#include "log.h"
#include "global.h"
#include "util.h"
#include "open_close.h"

#include <stdio.h>
#include <string.h>

int my_read(int fd, char* out_buf, size_t count)
{
    OFT* oft = running->fd[fd];
    if (!oft)
    {
        LOG("Error: FD %d is not open", fd);
        return -1;
    }

    F_MODE mode = oft->mode;
    int offset = oft->offset;
    
    if (mode != RD && mode != RW)
        return -1;

    INODE* ip = &oft->mip->INODE;

    if (offset >= ip->i_size)
        return -1;

    const size_t available = ip->i_size - offset;
    if (count > available)
        count = available;

    size_t n = 0;
    while (count)
    {
        const u32 log_blk = offset / BLKSIZE;
        const u32 blk = map(ip, log_blk, 0);
        const size_t start = offset % BLKSIZE;

        char buf[BLKSIZE];
        char* cp = get_block(blk, buf) + start;

        const size_t remainder = BLKSIZE - start;
        const size_t nBytes = min(count, remainder);

        LOG("offset=%d log_blk=%u blk=%u: Copying %u bytes", offset, log_blk, blk, nBytes);
        memcpy(out_buf, cp, nBytes);
        
        out_buf += nBytes;
        n += nBytes;
        offset += nBytes;

        count -= nBytes;
    }
    oft->offset = offset;

    LOG("Read %u bytes from FD %d", n, fd);
    return n;
}

void cmd_cat(char* filename)
{
    LOG("Cat: %s", filename);

    int fd = my_open(filename, RD);
    if (fd == -1)
    {
        printf("Could not open %s", filename);
        return;
    }
    
    char text_buf[513];
    int n;
    while ((n = my_read(fd, text_buf, 512)) > 0)
    {
        text_buf[n] = '\0';
        printf("%s", text_buf);
    }
    printf("%c", '\n');

    my_close(fd);
}
