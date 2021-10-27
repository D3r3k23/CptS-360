#ifndef CD_LS_PWD_H
#define CD_LS_PWD_H

#include "type.h"

int cd();
int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
int ls();
char *pwd(MINODE *wd);

#endif // CD_LS_PWD_H
