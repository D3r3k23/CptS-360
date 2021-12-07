#ifndef WRITE_CP_H
#define WRITE_CP_H

#include <stddef.h>

int my_write(int fd, char* in_buf, size_t count);
void cmd_cp(char* dest, char* src);
void cmd_mv(char* dest, char* src);

#endif // WRITE_CP_H
