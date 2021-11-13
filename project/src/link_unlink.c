#include "link_unlink.h"
#include "type.h"

#include "log.h"
#include "global.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <stdlib.h>


//link_unlink: link, unlink,symlink, readlink

void cmd_link(char *old_file, char *new_file)
{
	//veriy old_file exists and is not a DIR
	int oino = getino(old_file); //get old inode num
	if (!oino)
	{
		printf("Error: old_file does not exist\n");
		return 0;
	}
	MINODE *omip = iget(oino);
	//check omip->INODE file type (must not be DIR)
	if(S_ISDIR(omip->INODE.i_mode))
	{
		printf("Error: old_file is a DIR");
		iput(omip);
		return 0;
	}
	if(getino(new_file)==0)
	{
		//creat new_file with the same inode number of oldfile:
		char *parent = dirname(new_file);
		char *child = basename(new_file);
		int pino = getino(parent);

		MINODE *pmip = iget(pino);
		//creat entry in new parent DIR with same inode numer of old_file
		enter_name(pmip,oino,child);
		omip->INODE.i_links_count++; //inc INODE's links_count by 1
		omip->dirty = 1; //for write back by iput(omip)
		iput(omip);
		iput(pmip);
		}
	}
}

void cmd_unlink(char * filename)
{
	int ino = getino(filename);
	if(!ino) // check if inode exists
	{
		printf("path does not exist\n");
		return 0;
	}
	MINODE *mip = iget(ino);
	//check its a REG or symbolic LNK file; cannot be a DIR
	if(S_ISDIR(mip->INODE.i_mode))
	{
		printf("Error: file cannot be a DIR");
		iput(mip);
		return 0;
	}	

	//decrement INODE's link_count by 1
	mip->INODE.i_links_count--;
	if(mip->INODE.i_links_count > 0)
	{
		mip->dirty = 1; //for write INODE back to disk
	}
	else {
		//if links_count = 0: remove filename
		//deallocate all data blocks in INODE;
		//deallocate INODE;
		
		
	}
	char *parent = dirname(filename);
	char *child = basename(filename);
	int pino = getino(parent);
	MINODE *pmip = iget(pino);
	rm_child(pmip,ino,child);
	pmip->dirty = 1;
	iput(pmip);
	iput(mip); //release mip


}
