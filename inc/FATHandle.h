#ifndef FAT_HANDLE_H
#define FAT_HANDLE_H
#include "../inc/FATBoot.h"
typedef struct
{
    int fd;
    FAT_BootSector bs;
} FATHandle;
void fat_fs_mount(const char* device, FATHandle* fat_fs);
void fat_fs_unmount(FATHandle* fat_fs);
#endif // FAT_HANDLE_H