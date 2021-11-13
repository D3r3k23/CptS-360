#ifndef CD_LS_PWD_H
#define CD_LS_PWD_H

#include "type.h"

void cmd_cd(char* pathname);

void cmd_ls(char* pathname);
void ls_dir(MINODE* mip);
void ls_file(MINODE* mip, char* name);

void cmd_pwd();
void rec_pwd(MINODE* wd);

#endif // CD_LS_PWD_H
