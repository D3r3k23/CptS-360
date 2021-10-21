#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>

typedef enum
{
    EXIT,
    GET,
    PUT,
    LS,
    CD,
    PWD,
    MKDIR,
    RMDIR,
    RM,
    LCAT,
    LLS,
    LCD,
    LPWD,
    LMKDIR,
    LRMDIR,
    LRM,
    CMD_COUNT,
    CMD_NONE
} CMD;

CMD find_cmd(const char* cmd);

void c_cat(const char* pathname, FILE* f);
void c_ls(const char* pathname, FILE* f);
void c_cd(const char* pathname);
void c_pwd(FILE* f);
void c_mkdir(const char* pathname);
void c_rmdir(const char* pathname);
void c_rm(const char* pathname);

void ls_dir(const char* pathname, FILE* f);
void ls_file(const char* pathname, FILE* f);

#endif // COMMANDS_H
