#include "../inc/FATBoot.h"
#include "../inc/FATHandle.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// fd bs
void fat_fs_mount(const char* device, FATHandle* fat_fs)
{
    int fd = open(device, O_RDONLY);
    if (fd < 0)
    {
        perror("open error");
        abort();
    }
    FAT_BootSector bs;
    lseek(fd, 0, SEEK_SET);
    read(fd, &bs, sizeof(bs));
    fat_fs->bs = bs;
    fat_fs->fd = fd;
}
void fat_fs_unmount(FATHandle* fat_fs)
{
    close(fat_fs->fd);
}