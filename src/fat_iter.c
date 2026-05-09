#define _FILE_OFFSET_BITS 64
#define ITER_NOT_FIND -1
#define ITER_OK 0
#include "../inc/DIR.h"
#include "../inc/FATEntry.h"
#include "../inc/FATHandle.h"
#include "../inc/FATIter.h"
#include "../inc/FATLDIR.h"
#include "../inc/fat_calc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint64_t get_seek_sz_from_iter(FATIter* iter)
{
    return get_base_cluByte_seek_byte_address(
        &iter->fat_fs.bs, iter->cur_clu, iter->cur_byte);
}
// 原始的,不能处理跨簇,通过传入read_cnt 读取entry的个数
// 会自动移动cur_bytes
void fat_read_raw_dir_entry(FATIter* iter,
                            DIREntry* entries,
                            size_t read_cnt)
{
    FAT_BootSector* bs = &iter->fat_fs.bs;
    int fd = iter->fat_fs.fd;
    uint64_t seek_sz = get_seek_sz_from_iter(iter);
    lseek(fd, seek_sz, SEEK_SET);
    read(fd, entries, sizeof(DIREntry) * read_cnt);
    iter->cur_byte += sizeof(DIREntry) * read_cnt;
}
// 返回值如果是 UINT32_MAX 的话证明是没有找到的
uint32_t fat_get_next_clu(FATHandle* fat_fs,
                          uint32_t cur_clu)
{
    if (get_total_clusters(&fat_fs->bs) <= cur_clu)
    {
        return UINT32_MAX;
    }
    // fat_table 在BPB的位置是这个
    uint64_t fat_t_seek_sz = fat_fs->bs.BPB_RsvdSecCnt *
                             fat_fs->bs.BPB_BytsPerSec;
    // 移动到seek_sz
    lseek(fat_fs->fd,
          fat_t_seek_sz + cur_clu * sizeof(uint32_t),
          SEEK_SET);
    //
    uint32_t next_clu;
    read(fat_fs->fd, &next_clu, sizeof(next_clu));
    // 如果到eoc的话就返回追踪完了,没有东西
    if (next_clu <= 2 || next_clu > 0x0FFFFFF8)
    {
        return UINT32_MAX;
    }
    return next_clu;
}

// 当前不支持跨两个簇的情况,最多跨一个簇
int fat_read_can_jmp_clu_dir_entry(FATIter* iter,
                                   DIREntry* entries,
                                   size_t read_cnt)
{
    uint32_t total_clu_bytes = iter->per_clu_byte_size;
    uint32_t cur_bytes = iter->cur_byte;

    // 当前簇剩余字节数
    uint32_t remaining_bytes = total_clu_bytes - cur_bytes;
    // 当前簇剩余可读的条目数（按字节计算）
    uint32_t cur_clu_remaining_entries =
        remaining_bytes / sizeof(DIREntry);

    if (read_cnt <= cur_clu_remaining_entries)
    {
        // 不需要跨簇
        fat_read_raw_dir_entry(iter, entries, read_cnt);
        return ITER_OK;
    }
    else
    {
        // 需要跨簇：先读当前簇剩余部分
        fat_read_raw_dir_entry(iter, entries,
                               cur_clu_remaining_entries);
        entries += cur_clu_remaining_entries;

        // 需要读的下个簇的条目数
        uint32_t next_clu_read_cnt =
            read_cnt - cur_clu_remaining_entries;

        // 获取下一个簇
        uint32_t next_clu =
            fat_get_next_clu(&iter->fat_fs, iter->cur_clu);
        if (next_clu ==
            UINT32_MAX) // 注意：这里应该是 UINT32_MAX
        {
            return ITER_NOT_FIND;
        }

        // 切换到下一个簇
        iter->cur_clu = next_clu;
        iter->cur_byte = 0;

        // 读取下个簇的部分
        fat_read_raw_dir_entry(iter, entries,
                               next_clu_read_cnt);
        return ITER_OK;
    }
}
// 将 UTF-16 字符转换为 UTF-8，返回写入的字节数
int utf16_to_utf8(uint16_t unicode, char* output)
{
    if (unicode < 0x80)
    {
        // 1字节 ASCII
        output[0] = (char)unicode;
        return 1;
    }
    else if (unicode < 0x800)
    {
        // 2字节
        output[0] = 0xC0 | (unicode >> 6);
        output[1] = 0x80 | (unicode & 0x3F);
        return 2;
    }
    else
    {
        // 3字节
        output[0] = 0xE0 | (unicode >> 12);
        output[1] = 0x80 | ((unicode >> 6) & 0x3F);
        output[2] = 0x80 | (unicode & 0x3F);
        return 3;
    }
}
size_t fat_link_ldir_name(char* out_name,
                          LDIREntry* entries,
                          size_t ldir_len)
{
    char* ptr = out_name;
    size_t total_len = 0;

    // 定义每个name部分的长度
    struct NamePart
    {
        uint8_t* data;
        int count;
    } parts[] = {{NULL, 5}, {NULL, 6}, {NULL, 2}};

    for (int i = ldir_len - 1; i >= 0; i--)
    {
        LDIREntry* entry = &entries[i];
        parts[0].data = entry->LDIR_Name1;
        parts[1].data = entry->LDIR_Name2;
        parts[2].data = entry->LDIR_Name3;

        for (int p = 0; p < 3; p++)
        {
            uint16_t* chars = (uint16_t*)parts[p].data;
            for (int j = 0; j < parts[p].count; j++)
            {
                uint16_t ch = chars[j];
                if (ch == 0x0000)
                    goto done;
                if (ch == 0xFFFF)
                    continue;
                ptr += utf16_to_utf8(ch, ptr);
                total_len++;
            }
        }
    }

done:
    *ptr = '\0';
    return total_len;
}
int fat_get_long_dir_len(LDIREntry* first_ldir)
{
    return first_ldir->LDIR_Ord & 0x1F;
}
int fat_read_long_dir_entry(FATIter* fat_iter,
                            LDIREntry* long_entries,
                            LDIREntry* first_ldir,
                            uint32_t total_entries)
{
    if ((first_ldir->LDIR_Ord & 0x40) == 0)
    {
        return ITER_NOT_FIND;
    }
    if (total_entries == 0 || total_entries > 20)
    { // 最多20个条目（255字符）
        return 1;
    }
    if (!long_entries)
        return ITER_NOT_FIND;
    long_entries[0] = *first_ldir;
    //---
    int success = 1;
    for (uint8_t ord = 2; ord <= total_entries; ord++)
    {
        DIREntry next_entry;
        if (fat_read_can_jmp_clu_dir_entry(
                fat_iter, &next_entry, 1) != ITER_OK)
        {
            success = 0;
            break;
        }

        LDIREntry* current = (LDIREntry*)&next_entry;
        uint8_t current_ord = current->LDIR_Ord & 0x1F;

        long_entries[ord - 1] = *current;
    }
    return ITER_OK;
}
void fat_iter_init(FATIter* fat_iter, FATHandle* fat_fs,
                   uint32_t start_clu)
{
    fat_iter->cur_clu = start_clu;
    fat_iter->cur_byte = 0;
    fat_iter->per_clu_byte_size =
        fat_fs->bs.BPB_BytsPerSec *
        fat_fs->bs.BPB_SecPerClus;
    fat_iter->fat_fs = *fat_fs;
}
// 当前是指向的 (cur_clu,clu_byte)
int fat_iter_next(FATIter* fat_iter, DIR* dp_out)
{
    DIREntry entry;
    dp_out->name_len = 0;
    while (1)
    {
        fat_read_can_jmp_clu_dir_entry(fat_iter, &entry, 1);
        // 如果当前文件被删除的话,就跳过
        if (entry.DIR_Name[0] == DIR_EMPTY)
        {
            continue;
        }
        // 如果后面没有文件的话,就返回没有找到
        if (entry.DIR_Name[0] == DIR_EMPTY_ALL)
        {
            return ITER_NOT_FIND;
        }
        if (entry.DIR_Attr == ATTR_LONG_NAME)
        {
            LDIREntry* first_ldir = (LDIREntry*)&entry;

            uint32_t total_entries =
                fat_get_long_dir_len(first_ldir);
            LDIREntry* long_entries =
                malloc(sizeof(LDIREntry) * total_entries);
            fat_read_long_dir_entry(fat_iter, long_entries,
                                    first_ldir,
                                    total_entries);
            // 注意：long_entries[0]是序号1（最后一个），long_entries[total_entries-1]是序号N（第一个）
            size_t write_len = fat_link_ldir_name(
                dp_out->name, long_entries, total_entries);
            dp_out->name_len = write_len;
            free(long_entries);
            continue;
        }
        dp_out->dir = entry;
        if (dp_out->name_len == 0)
        {
            memcpy(dp_out->name, dp_out->dir.DIR_Name, 11);
            dp_out->name_len = 11;
        }
        return ITER_OK;
    }
}

#ifdef _DEBUG
#include "../inc/FATHandle.h"
#include "../inc/FATLDIR.h"
#define DEV "./setup_bash/fat32.img"

void test_iter1()
{
    FATHandle fat_fs;
    FATIter iter;
    DIR entry;
    fat_fs_mount(DEV, &fat_fs);
    fat_iter_init(&iter, &fat_fs, fat_fs.bs.BPB_RootClus);
    while (fat_iter_next(&iter, &entry) != ITER_NOT_FIND)
    {
        printf("%.*s\n", entry.name_len, entry.name);
    }
    putchar('\n');
    fat_fs_unmount(&fat_fs);
}
void print_clu_size()
{
    FATHandle fat_fs;
    fat_fs_mount(DEV, &fat_fs);
    printf("clu_size=%d", fat_fs.bs.BPB_SecPerClus *
                              fat_fs.bs.BPB_BytsPerSec);
}
void test_lseek()
{
}
#endif