#include "file_transfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#define MAX 256

void send_file(int sock, const char* pathname)
{
    // 1. Stat file
    struct stat st;
    int r = lstat(pathname, &st);
    if (r == -1)
    {
        printf("Unable to stat %s\n", pathname);
        return;
    }
    // Send file mode?

    char buf[MAX];

    // 2. Send file size
    int file_size = st.st_size;
    sprintf(buf, "%d", file_size);
    write(sock, buf, MAX);

    // 3. Read file and send contents
    int fd = open(pathname, O_RDONLY);
    if (fd >= 0)
    {
        int total = 0;
        while (total < file_size)
        {
            memset(buf, 0, MAX);
            int n = read(fd, buf, MAX);
            write(sock, buf, n);
            total += n;
        }
    close (fd);
    }
}

void recv_file(int sock, const char* pathname)
{
    char buf[MAX];

    // 1. Receive file size
    int n = read(sock, buf, MAX);
    int file_size = strtol(buf, NULL, 10);

    // 2. Create file
    int fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    // 3. Receive file and save contents
    int total = 0;
    while (total < file_size)
    {
        memset(buf, 0, MAX);
        int n = read(sock, buf, MAX);
        write(fd, buf, n);
        total += n;
    }
    close (fd);
}
