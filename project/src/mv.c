#include "mv.h"

int mywrite(int fd, char buf[ ], int nbytes) 
{
  while (nbytes > 0 ){

     compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

          lbk       = oftp->offset / BLKSIZE;
          startByte = oftp->offset % BLKSIZE;

    // I only show how to write DIRECT data blocks, you figure out how to 
    // write indirect and double-indirect blocks.

     if (lbk < 12){                         // direct block
        if (ip->INODE.i_block[lbk] == 0){   // if no data block yet

           mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
        }
        blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
     }
     else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
              // HELP INFO:
              if (i_block[12] == 0){
                  allocate a block for it;
                  zero out the block on disk !!!!
              }
              get i_block[12] into an int ibuf[256];
              blk = ibuf[lbk - 12];
              if (blk==0){
                 allocate a disk block;
                 record it in i_block[12];
              }
              .......
     }
     else{
            // double indirect blocks */
     }

     /* all cases come to here : write to the data block */
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (offset > INODE.i_size)  // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset > fileSize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to outer while to write more .... until nbytes are written
  }

  mip->dirty = 1;       // mark mip dirty for iput() 
  printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
  return nbytes;
}

