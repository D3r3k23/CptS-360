#ifndef READ_CAT_H
#define READ_CAT_H

#include "type.h"

#include <stddef.h>

int my_read(int fd, char* out_buf, size_t count);
u32 map(INODE* ip, u32 log_blk);

void cmd_cat(char* filename);

#endif // READ_CAT_H
