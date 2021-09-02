#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct
{
    u8 drive;

    u8 head;
    u8 sector;
    u8 cylinder;

    u8 sys_type;

    u8 end_head;
    u8 end_sector;
    u8 end_cylinder;

    u32 start_sector;
    u32 nr_sectors;
} partition;

const char* dev = "vdisk";
const u32 ptable_offset = 0x1be;

int read_sector(int fd, int sector, char* buf)
{
    lseek(fd, sector * 512, SEEK_SET);
    read(fd, buf, 512);
}

int main()
{
    char buf[512];
    partition* p;

    u32 extStart;

    int fd = open(dev, O_RDONLY);
    read_sector(fd, 0, buf); // MBR sector
    p = (partition*)&buf[ptable_offset]; // Start of partition table in MBR

    printf("partition start_sector nr_sectors sys_type\n");

    int i;
    for (i = 1; i <= 4; i++)
    {
        printf("P%d: %14u %10u %8x\n", i, p->start_sector, p->nr_sectors, p->sys_type);

        if (i == 4) // Assume P4 is EXT
            extStart = p->start_sector;
        
        p++; // Advance to next partition in MBR table
    }
    printf("\n");
    printf("****** Extended partitions ******\n");
    printf("partition start_sector nr_sectors\n");

    u32 localMBR = extStart;
    while (1)
    {
        // Read and print first entry in localMBR table
        read_sector(fd, localMBR, buf);
        p = (partition*)&buf[ptable_offset];
        printf("P%d: %14u %10u\n", i, localMBR + p->start_sector, p->nr_sectors);

        p++; // Advance to second entry in localMBR table
        
        // Advance to next localMBR if it is a partition, break otherwise
        if (p->sys_type)
            localMBR = extStart + p->start_sector;
        else
            break;
        i++;
    }

    return 0;
}
