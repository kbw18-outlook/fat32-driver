#ifndef FAT_ITER_H
#define FAT_ITER_H
#include "../inc/DIR.h"
#include "../inc/FATHandle.h"
#include <stdint.h>
typedef struct
{
    uint32_t cur_clu;
    uint32_t
        cur_byte; // 相对于clu的bytes 而不是对于开头的bytes!
    uint32_t per_clu_byte_size;
    FATHandle fat_fs;

} FATIter;
void fat_iter_init(FATIter* fat_iter, FATHandle* fat_fs,
                   uint32_t start_clu);
int fat_iter_next(FATIter* fat_iter, DIR* dp_out);
#endif // FAT_ITER_H