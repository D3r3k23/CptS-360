#include "global.h"

MINODE minode[NMINODE];
MINODE* root;
PROC proc[NPROC];
PROC* running;

char gpath[128]; // global for tokenized components
char* name[64];  // assume at most 64 components in pathname

int dev=0;
int nblocks=0, ninodes=0, bmap, imap=0, iblk=0;

char line[128], cmd[64], pathname[128];
