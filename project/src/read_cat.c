#include "read_cat.h"

#include "log.h"
#include "global.h"
#include "util.h"
#include "open_close.h"

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

    MINODE* mip = oft->mip;
    INODE* ip = &mip->INODE;

    if (offset >= ip->i_size)
        return -1;

    size_t available = ip->i_size - offset;
    u32 n = 0;

    while (count && available)
    {
        u32 log_blk = offset / BLKSIZE;
        u32 start = offset % BLKSIZE;
        u32 blk = map(ip, log_blk);

        char buf[BLKSIZE];
        get_block(blk, buf);

        char* cp = buf + start;
        u32 remainder = BLKSIZE - start;

        u32 nBytes = min(count, remainder);

        LOG("offset=%d log_blk=%u", offset, log_blk);
        LOG("Copying %u bytes from blk %u", nBytes, blk);
        memcpy(out_buf, cp, nBytes);
        
        out_buf += nBytes;
        n += nBytes;
        offset += nBytes;

        count -= nBytes;
        available -= nBytes;
        for (int i = 0; i < 20000000; i++);
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
