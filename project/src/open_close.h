#ifndef OPEN_CLOSE_H
#define OPEN_CLOSE_H

#include "type.h"

int my_open(char* filename, F_MODE mode);
void truncate(MINODE* mip);
OFT* make_oft(MINODE* mip, F_MODE mode);

int my_close(int fd);

int my_lseek(int fd, u32 position);

void cmd_pfd(void);

#endif // OPEN_CLOSE_H
