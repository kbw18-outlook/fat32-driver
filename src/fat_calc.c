#include "../inc/FATBoot.h"
#include <stdint.h>
// 作用：获取单个 FAT 表占用了多少个扇区。
uint32_t
get_single_fat_table_sector(const FAT_BootSector* bs)
{
    if (bs->BPB_FATSz16 != 0)
    {
        return bs->BPB_FATSz16; // FAT12/FAT16
    }
    else
    {
        return bs->BPB_FATSz32; // FAT32
    }
}
// fat32没有 root dir 所以是0
uint32_t calc_root_dir_sectors(const FAT_BootSector* bs)
{
    if (bs->BPB_RootEntCnt == 0)
    {
        return 0; // FAT32
    }

    uint32_t root_dir_bytes = bs->BPB_RootEntCnt * 32;
    uint32_t sectors =
        (root_dir_bytes + bs->BPB_BytsPerSec - 1) /
        bs->BPB_BytsPerSec;
    return sectors;
}
// 返回 3区域(data)扇区编号
uint32_t get_data_sector_idx(const FAT_BootSector* bs)
{
    return bs->BPB_RsvdSecCnt +
           (bs->BPB_NumFATs *
            get_single_fat_table_sector(bs)) +
           calc_root_dir_sectors(bs);
}
// 返回 编号为n簇的扇区数
uint32_t
get_index_sector_of_cluster(const FAT_BootSector* bs,
                            uint32_t n_cluster)
{
    return ((n_cluster - 2) * bs->BPB_SecPerClus) +
           get_data_sector_idx(bs);
}
// 返回总扇区数
uint32_t get_total_sectors(const FAT_BootSector* bs)
{
    if (bs->BPB_TotSec16 != 0)
        return bs->BPB_TotSec16;
    else
        return bs->BPB_TotSec32;
}
// 返回data的扇区数
uint32_t get_data_total_sectors(const FAT_BootSector* bs)
{
    uint32_t fatSz = get_single_fat_table_sector(bs);
    uint32_t rootDirSectors = calc_root_dir_sectors(bs);
    uint32_t totSec = get_total_sectors(bs);

    return totSec -
           (bs->BPB_RsvdSecCnt + (bs->BPB_NumFATs * fatSz) +
            rootDirSectors);
}
// 返回总的簇数
uint32_t get_total_clusters(const FAT_BootSector* bs)
{
    return get_data_total_sectors(bs) / bs->BPB_SecPerClus;
}
// 寻找当前start_clu中,在fat表上最多需要多少簇,方便分配空间
uint32_t get_data_clu_cnt(const uint32_t* fat,
                          uint32_t start_clu)
{
    int cur_clu = start_clu;
    int cnt = 0;
    while (cur_clu >= 2 && cur_clu < 0x0FFFFFF8)
    {
        cnt++;
        cur_clu = fat[cur_clu];
    }
    return cnt;
}
uint64_t
get_base_cluByte_seek_byte_address(const FAT_BootSector* bs,
                                   uint32_t base_clu,
                                   uint32_t after_bytes)
{
    return get_index_sector_of_cluster(bs, base_clu) *
               bs->BPB_BytsPerSec +
           after_bytes;
}