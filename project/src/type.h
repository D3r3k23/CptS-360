#ifndef TYPE_H
#define TYPE_H

#include <ext2fs/ext2_fs.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define FREE  0
#define READY 1

#define BLKSIZE  1024
#define NMINODE  128
#define NPROC    2
#define NFD      16

typedef enum f_mode
{
    RD = 0,
    WR = 1,
    RW = 2,
    AP = 3
} F_MODE;

typedef struct minode
{
    INODE INODE;  // INODE structure on disk
    int dev, ino; // (dev, ino) of INODE
    int refCount; // in use count
    int dirty;    // 0 for clean, 1 for modified

    int mounted;          // for level-3
    struct mntable *mptr; // for level-3
} MINODE;

typedef struct oft // Open file table
{
    F_MODE mode;
    int refCount;
    MINODE* mip;
    int offset;
} OFT;

typedef struct proc
{
    struct proc* next;
    int          pid;  // process ID
    int          ppid; // parent ID?
    int          status;
    int          uid; // user ID
    int          gid;
    MINODE*      cwd; // CWD directory pointer
    OFT*         fd[NFD];
} PROC;

#endif // TYPE_H
