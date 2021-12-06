#ifndef RMDIR_H
#define RMDIR_H

#include "type.h"

void cmd_rmdir(char* pathname);

void rm_child(MINODE* pmip, char* name);

#endif // RMDIR_H
