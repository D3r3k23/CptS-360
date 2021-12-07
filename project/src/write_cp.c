#include "write_cp.h"

#include "log.h"
#include "global.h"
#include "util.h"
#include "open_close.h"

#include <string.h>

int my_write(int fd, char* in_buf, size_t count)
{

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
