#ifndef FS_INFO_H
#define FS_INFO_H
#include <stdint.h>
typedef struct __attribute__((packed))
{
    uint32_t FSI_LeadSig;       // 0x41615252
    uint8_t FSI_Reserved1[480]; // 0
    uint32_t FSI_StrucSig;      // 0x61417272
    uint32_t
        FSI_Free_Count; // 有用  0xFFFFFFFF 代表需要重新计算
    uint32_t FSI_Nxt_Free;     // 有用
    uint8_t FSI_Reserved2[12]; // 0
    uint32_t FSI_TrailSig;     // 0xAA550000
} FSInfo;
#endif // FS_INFO_H