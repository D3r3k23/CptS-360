#ifndef MKDIR_CREAT_H
#define MKDIR_CREAT_H

#include "type.h"

void cmd_mkdir(char* pathname);
void mkdir_impl(MINODE* pmip, char* name);

void cmd_creat(char* pathname);
void creat_impl(MINODE* pmip, char* name);

void enter_name(MINODE* pmip, u32 ino, char* name);

#endif // MKDIR_CREAT_H
