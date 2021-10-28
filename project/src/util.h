#ifndef UTIL_H
#define UTIL_H

#include "type.h"

char* get_block(int blk, char* buf);
int put_block(int blk, char* buf);

MINODE *iget(u32 ino);
void iput(MINODE *mip);
u32 search(MINODE *mip, char *name);
u32 getino(char *pathname);
int findmyname(MINODE *parent, u32 my_ino, char my_name[]);
u32 findino(MINODE* mip, u32* my_ino);

int tokenize(char *pathname);
int streq(const char* s1, const char* s2);
int min(int a, int b);
int max(int a, int b);

#endif // UTIL_H
