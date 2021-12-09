#ifndef GLOBAL_H
#define GLOBAL_H

#include "type.h"

// Defined in main.c

extern MINODE minode[NMINODE];
extern MINODE* root;
extern PROC proc[NPROC];
extern PROC* running;

extern char gpath[128]; // global for tokenized components
extern char* name[64];  // assume at most 64 components in pathname

extern int dev;
extern int nblocks, ninodes, bmap, imap, iblk;

#define SUPER_INO 1
#define ROOT_INO 2

#define SUPER_USER 0

#endif // GLOBAL_H
