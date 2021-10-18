#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum
{
    EXIT = 0,
    GET,
    PUT,
    LS,
    CD,
    PWD,
    MKDIR,
    RMDIR,
    RM,
    LEXIT,
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

void c_cat(const char* pathname);
void c_ls(const char* pathname);
void c_cd(const char* pathname);
void c_pwd(void);
void c_mkdir(const char* pathname);
void c_rmdir(const char* pathname);
void c_rm(const char* pathname);

void ls_dir(const char* pathname);
void ls_file(const char* pathname);

#endif // COMMANDS_H
