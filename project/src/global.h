#ifndef GLOBAL_H
#define GLOBAL_H

#include "type.h"

extern MINODE minode[NMINODE];
extern MINODE* root;
extern PROC proc[NPROC];
extern PROC* running;

extern char gpath[128]; // global for tokenized components
extern char* name[64];  // assume at most 64 components in pathname
extern int n;           // number of component strings

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

#endif // GLOBAL_H
