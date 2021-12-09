#include "cp.h"

int write_file()
{
  1. Preprations:
     ask for a fd   and   a text string to write;

  2. verify fd is indeed opened for WR or RW or APPEND mode

  3. copy the text string into a buf[] and get its length as nbytes.

     return(mywrite(fd, buf, nbytes));
}
