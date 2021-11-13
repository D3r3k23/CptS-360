#include "symlink.h"


void cmd_symlink(char * old_file, char* new_file)
{
	//check oldfile must exist and new_file not yet exist
	// new_file; change new_file to LNK type;
	int oino = getino(old_file);
	if(!oino) //check if inode exists
	{
		printf("Error: source path does not exist\n");
		return 0;
	}

	int ino = getino(new_file);
	MINODE *mip = iget(ino);
	mip->INODE.i_mode = 0xA1A4; //new file to LNK type

	//assume length of old_file name <= 60 chars
	//store old_file name in newfile's INODE.i_block[] area
	//set file size to length of old_file name
	//mark new_file's minode dirty;
	iput(mip);
	char *parent = dirname(new_file);
	char *child = basename(new_file);
	int pino = getino(parent);
	MINODE *pmip = iget(pino);
	rm_child(pmip,ino,child);
	pmip->dirty = 1; //mark new_file parent minode dirty;
	iput(pmip);
	iput(pino);
}

void cmd_readlink(char *pathname)
{
//get file's INODE in memory, verift its a LNK file
//copy targets filename from INODE.i_block[] into buffer;
//return file size
	int ino = getino(pathname);
		if(!ino) //checks if the inode exists
		{
			printf("source path does not exist\n");
			return 0;
		}
	
		MINODE *mip = iget(ino);
	
		if(!S_ISLNK(mip->INODE.i_mode)) //checks if the inode is a LNK
		{
			printf("path is not a LNK\n"); //if the inode is not a LNK file, return
			return 0;
		}
	
		printf("%s\n", (char *) (mip->INODE.i_block));
	
		iput(mip);
	
}
