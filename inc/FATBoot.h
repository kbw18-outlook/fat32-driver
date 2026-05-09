#ifndef FAT_BOOT_H
#define FAT_BOOT_H
#include <stdint.h>
/*
vfat 是小端序的
卷的四个基本区域布局:
    0   Reserved Region（保留区域） 有BPB
    1   FAT Region(FAT 表区域) 通常会有两份（FAT1
        和FAT2，互为备份)
    2   Root Directory Region（根目录区域） FAT32没有
    3   File and Directory Data
        Region（文件与目录数据区域）存储实际的文件内容、以及子目录的条目（FAT32
        中包括根目录）。数据按“簇”分配。
-------
BPB:
    位置：位于卷的 第一个扇区（0 号扇区），属于保留区域
    作用：BPB 是 FAT
        卷最核心的元数据，记录了文件系统的基本参数
        （如每扇区字节数、每簇扇区数、FAT
        表大小、总扇区数等）。
*/
/*      1   2       3-5     6
0 区域  BPB     |fsInfo| ...   |table_back|
1 区域  fat_table
3 区域   |簇2|  |簇3|   |簇4|  ... |簇get_total_clusters |
簇2代表根目录BPB_RootClus
*/
typedef struct __attribute__((packed))
{
    uint8_t BS_jmpBoot[3];   // 保留
    uint8_t BS_OEMName[8];   // 保留
    uint16_t BPB_BytsPerSec; // 512 扇区多少byte
    uint8_t BPB_SecPerClus;  // 每簇扇区数
    uint16_t BPB_RsvdSecCnt; // 32
    uint8_t BPB_NumFATs;     // 2
    uint16_t BPB_RootEntCnt; // 0
    uint16_t BPB_TotSec16;   // 0
    uint8_t BPB_Media; // 0xF0, 0xF8, 0xF9, 0xFA, 0xFB,
                       // 0xFC, 0xFD, 0xFE, 0xFF
    //
    uint16_t BPB_FATSz16;   // 0
    uint16_t BPB_SecPerTrk; // 63
    uint16_t BPB_NumHeads;  // 255
    uint32_t BPB_HiddSec;   // 0
    uint32_t BPB_TotSec32;  // 总扇区数
    uint32_t BPB_FATSz32;   // 每个FAT表的真实扇区数
    uint16_t BPB_ExtFlags;  // 0
    uint16_t BPB_FSVer;     // 0
    uint32_t BPB_RootClus;  // 2 代表第一个簇
    uint16_t BPB_FSInfo;    // 1
    uint16_t BPB_BkBootSec; // 6
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig; // 0x29
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8]; // "FAT32   "

} FAT_BootSector;
#endif // FAT_BOOT_H