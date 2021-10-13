typedef enum
{
    GET = 0,
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

const char* COMMANDS[CMD_COUNT] = 
{
    "get",
    "put",
    "ls",
    "cd",
    "pwd",
    "mkdir",
    "rmdir",
    "rm",
    "lcat",
    "lls",
    "lcd",
    "lpwd",
    "lmkdir",
    "lrmdir",
    "lrm"
};

CMD find_cmd(const char* cmd);

void c_cat(const char* pathname);
void c_ls(const char* pathname);
void c_cd(const char* pathname);
void c_pwd(void);
void c_mkdir(const char* pathname);
void c_rmdir(const char* pathname);
void c_rm(const char* pathname);

void c_ls_file(const char* pathname);
void c_ls_dir(const char* pathname);
