#ifndef DIR_H
#define DIR_H
#include "FATEntry.h"
#include <stdint.h>
typedef struct
{
    char name[256];
    uint32_t name_len;
    DIREntry dir;
} DIR;
#endif // DIR_H