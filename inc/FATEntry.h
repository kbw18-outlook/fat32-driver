#ifndef FAT_ENTRY_H
#define FAT_ENTRY_H
#include <stdint.h>
typedef enum
{
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN = 0x02,
    ATTR_SYSTEM = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE = 0x20,
    ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN |
                     ATTR_SYSTEM | ATTR_VOLUME_ID
} Attr;
#define DIR_EMPTY 0xE5
#define DIR_EMPTY_ALL 0x00
typedef struct __attribute__((packed))
{
    uint8_t DIR_Name[11]; // short name
    uint8_t
        DIR_Attr; // 为什么Attr只有两位了,因为另外的需要保留,设置为0
    uint8_t DIR_NTRes; // 0
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} DIREntry;
#endif // FAT_ENTRY_H