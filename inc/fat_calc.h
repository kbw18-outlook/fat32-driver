#ifndef FAT_CALC_H
#define FAT_CALC_H
#include "./FATBoot.h"
uint32_t
get_single_fat_table_sector(const FAT_BootSector* bs);
uint32_t calc_root_dir_sectors(const FAT_BootSector* bs);
uint32_t get_data_sector_idx(const FAT_BootSector* bs);
uint32_t
get_index_sector_of_cluster(const FAT_BootSector* bs,
                            uint32_t n_cluster);
uint32_t get_total_sectors(const FAT_BootSector* bs);
uint32_t get_data_total_sectors(const FAT_BootSector* bs);
uint32_t get_total_clusters(const FAT_BootSector* bs);
uint32_t get_data_clu_cnt(const uint32_t* fat,
                          uint32_t start_clu);
uint32_t
get_base_cluByte_seek_byte_address(const FAT_BootSector* bs,
                                   uint32_t base_clu,
                                   uint32_t after_bytes);
#endif // FAT_CALC_H