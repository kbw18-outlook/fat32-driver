#ifndef FAT_L_DIR_H
#define FAT_L_DIR_H
#include <stdint.h>
typedef struct __attribute__((packed))
{
    uint8_t LDIR_Ord;
    uint8_t LDIR_Name1[10];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type;
    uint8_t LDIR_Chksum;
    uint8_t LDIR_Name2[12];
    uint16_t LDIR_FstClusLO; // 0
    uint8_t LDIR_Name3[4];
} LDIREntry;
#endif // FAT_L_DIR_H