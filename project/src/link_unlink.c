#include "link_unlink.h"

#include "type.h"

//link_unlink: link, unlink,symlink, readlink

int cmd_link(char *old_file, char *new_file)
{
	//veriy old_file exists and is not a DIR
	int oino = getino(old_file); //get old inode num
	if (!oino)
	{
		printf("Error: old_file does not exist\n");
		return 0;
	}
	MINODE *omip = iget(dev,oino);
	//check omip->INODE file type (must not be DIR)
	if(S_ISDIR(omip->INODE.i_mode))
	{
		printf("Error: old_file is a DIR");
		iput(omip);
		return 0;
	}
	else{
		if(getino(new_file)==0)
		{
		//creat new_file with the same inode number of oldfile:
			char *parent = dirname(new_file);
			char *child = basename(new_file);
			int pino = getino(parent);

			MINODE *pmip = iget(dev, pino);
			//creat entry in new parent DIR with same inode numer of old_file
			enter_name(pmip,oino,child);
			omip->INODE.i_links_count++; //inc INODE's links_count by 1
			omip->dirty = 1; //for write back by iput(omip)
			iput(omip);
			iput(pmip);
		}
	}
}

int cmd_unlink(char * filename)
{
	ino = getino(filename);
	if(!ino) // check if inode exists
	{
		printf("path does not exist\n");
		return 0;
	}
	MINODE *mip=iget(dev,ino);
	//check its a REG or symbolic LNK file; cannot be a DIR
	if(S_ISDIR(mip->inode.i_mode))
	{
		printf("Error: file cannot be a DIR");
		iput(mip);
		return 0;
	}
	else
	{
		//remove name entry from parents DIR data block
		char *parent = dirname(filename);
		char *child = basename(filename);
		int pino = ino(parent);
		MODE *pimp = iget(dev,pino);
		rm_child(pmip,ino,child);
		pmip->dirty = 1;
		iput(pmip);
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
	iput(mip); //release mip


}

int symlink(char * old_file, char* new_file)
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
	MINODE *mip = iget(dev, ino);
	mip->inode.i_mode = 0xA1A4; //new file to LNK type

	//assume length of old_file name <= 60 chars
	//store old_file name in newfile's INODE.i_block[] area
	//set file size to length of old_file name
	//mark new_file's minode dirty;
	iput(mip);
	char *parent = dirname(new_file);
	char *child = basename(new_file);
	int pino = ino(parent);
	MODE *pimp = iget(dev,pino);
	rm_child(pmip,ino,child);
	pmip->dirty = 1; //mark new_file parent minode dirty;
	iput(pmip);
	iput(pino);
}

int readlink(char *pathname)
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
	
		MINODE *mip = iget(dev, ino);
	
		if(!S_ISLNK(mip->inode.i_mode)) //checks if the inode is a LNK
		{
			printf("path is not a LNK\n"); //if the inode is not a LNK file, return
			return 0;
		}
	
		printf("%s\n", (char *) (mip->inode.i_block));
	
		iput(mip);
	
}

