#include "rmdir.h"

#include "log.h"
#include "util.h"

#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <libgen.h>


void cmd_rmdir(char* pathname)
{
    u32 ino = getino(pathname);
    MINODE* mip = iget(ino);
    if(access(pathname,mip->INODE.i_mode))
    {
    	    if (!S_ISDIR(mip->INODE.i_mode))
	    {
		printf("Error: %s is not a directory\n", pathname);
		return;
	    }

	    if (!is_empty(mip))
	    {
		printf("Error: %s is not empty\n", pathname);
		return;
	    }

	    for (int i = 0; i < 12; i++)
	    {
		if (mip->INODE.i_block[i])
		    bdalloc(mip->INODE.i_block[i]);
	    }
	    idalloc(mip->ino);

	    mip->dirty = 1;
	    iput(mip);

	    u32 pino = findino(mip, NULL);
	    MINODE* pmip = iget(pino);

	    char temp[256];
	    strcpy(temp, pathname);
	    char* name = basename(temp);

	    rm_child(pmip, name);

	    pmip->INODE.i_links_count--;
	    pmip->INODE.i_atime = time(NULL);
	    pmip->INODE.i_ctime = time(NULL);
	    pmip->dirty = 1;
	    iput(pmip);
    }

}

void rm_child(MINODE* pmip, char* name)
{
    DIR* prev_dp = NULL;
    INODE* ip = &pmip->INODE;

    for (int i = 0; i < 12; i++)
        if (ip->i_block[i])
        {
            char buf[BLKSIZE];
            char* cp = get_block(ip->i_block[i], buf);

            while (cp < buf + BLKSIZE)
            {
                DIR* dp = (DIR*)cp;

                char entry_name[256];
                strncpy(entry_name, dp->name, dp->name_len);
                entry_name[dp->name_len] = '\0';

                if (strcmp(entry_name, name) == 0)
                {
                    LOG("Name: %s found", entry_name);
                    if (cp == buf && cp + dp->rec_len == buf + BLKSIZE) // First & only entry in block
                    {
                        LOG("First & only entry in block");
                        bdalloc(pmip->INODE.i_block[i]);
                        ip->i_size -= BLKSIZE;

                        for (i++; i < 12; i++)
                        {
                            if (!ip->i_block[i])
                                break;
                            else
                            {
                                get_block(ip->i_block[i], buf);
                                put_block(ip->i_block[i - 1], buf);
                            }
                        }
                    }
                    else if (cp + dp->rec_len == buf + BLKSIZE) // Last entry in block
                    {
                        LOG("Last entry in block");
                        prev_dp->rec_len += dp->rec_len;
                        put_block(ip->i_block[i], buf);
                    }
                    else
                    {
                        LOG("Not first/last & not first & only");
                        char* temp = buf;
                        DIR* last_dp = (DIR*)temp;
                        while (temp + last_dp->rec_len < buf + BLKSIZE) // Find last entry in block
                        {
                            temp += last_dp->rec_len;
                            last_dp = (DIR*)temp;
                        }

                        last_dp->rec_len += dp->rec_len;

                        char* start_ptr = cp + dp->rec_len;
                        size_t size = (buf + BLKSIZE) - start_ptr;

                        memmove(cp, start_ptr, size);
                        put_block(ip->i_block[i], buf);
                    }

                    pmip->dirty = 1;
                    iput(pmip);
                    return;
                }

                prev_dp = dp;
                cp += dp->rec_len;
            }
        }

    LOG("Child %s not found", name);
}
