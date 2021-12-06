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

// extern char line[256], cmd[64], pathname1[128], pathname2[128];

extern const char* disk;

#define SUPER_INO 1
#define ROOT_INO 2

#endif // GLOBAL_H
