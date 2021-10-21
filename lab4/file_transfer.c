#include "file_transfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
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
        printf("Error: Unable to stat %s\n", pathname);
        return;
    }
    if (!S_ISREG(st.st_mode))
    {
        printf("Error: %s is not a file\n", pathname);
        return;
    }

    char buf[MAX];

    // 2. Send file size
    int file_size = st.st_size;
    sprintf(buf, "%d\n", file_size);
    write(sock, buf, MAX);

    memset(buf, 0, MAX);

    // 3. Send file mode
    int file_mode = st.st_mode;
    sprintf(buf, "%d", file_mode);
    write(sock, buf, MAX);

    // 4. Read file and send contents
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
    read(sock, buf, MAX);
    int file_size = strtol(buf, NULL, 10);

    memset(buf, 0, MAX);

    // 2. Receive file mode
    read(sock, buf, MAX);
    int file_mode = strtol(buf, NULL, 10);

    // 3. Create file
    int fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, file_mode);
    
    // 4. Receive file and save contents
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
