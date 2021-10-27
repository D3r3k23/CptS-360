#ifndef CD_LS_PWD_H
#define CD_LS_PWD_H

#include "type.h"

void cd();
void ls_file(MINODE *mip, char *name);
void ls_dir(MINODE *mip);
void ls();
void pwd(MINODE *wd);

#endif // CD_LS_PWD_H
