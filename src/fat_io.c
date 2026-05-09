#include "../inc/FATHandle.h"
#include "../inc/FATIter.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef FATIter my_dirent;
#define FILE_ERR -1
#define FILE_OK 0
// 如果是UINT32_MAX代表没有找到

uint32_t fat_find_base_path_file_clu(FATIter* cur_iter,
                                     const char* file_name)
{
    DIR dp;
    while (1)
    {
        if (fat_iter_next(cur_iter, &dp) != FILE_OK)
        {
            return UINT32_MAX;
        }
        size_t file_name_len = strlen(file_name);
        if (file_name_len != dp.name_len)
        {
            continue;
        }
        if (memcmp(file_name, dp.name, dp.name_len) == 0)
        {
            return (dp.dir.DIR_FstClusHI << 16 |
                    dp.dir.DIR_FstClusLO);
        }
    }
    return UINT32_MAX;
}
//   /a/b/c/d/
uint32_t fat_find_start_clu(const char* path,
                            FATHandle* fat_fs)
{
    uint32_t root_clu = fat_fs->bs.BPB_RootClus;
    if (path == NULL || fat_fs == NULL)
    {
        return UINT32_MAX;
    }

    // 处理根目录情况
    if (path[0] == '\0' ||
        (path[0] == '/' && path[1] == '\0'))
    {
        return root_clu;
    }

    // 跳过开头的 '/'
    const char* p = path;
    while (*p == '/')
    {
        p++;
    }
    if (*p == '\0')
    {
        return root_clu;
    }

    // 从根目录开始
    FATIter cur_iter;
    fat_iter_init(&cur_iter, fat_fs, root_clu);

    char component[256];

    while (*p)
    {
        // 提取路径组件
        int i = 0;
        while (*p && *p != '/' && i < sizeof(component) - 1)
        {
            component[i++] = *p++;
        }
        component[i] = '\0'; // 正确的位置：在循环结束后赋值

        // 跳过 '/'
        if (*p == '/')
        {
            p++;
        }

        // 跳过空组件（连续的斜杠）
        if (i == 0)
        {
            continue;
        }

        // 在当前目录中查找
        uint32_t found_cluster =
            fat_find_base_path_file_clu(&cur_iter,
                                        component);
        if (found_cluster == UINT32_MAX)
        {
            return UINT32_MAX; // 找不到
        }

        // 如果已经到达路径末尾，返回找到的簇
        if (*p == '\0')
        {
            return found_cluster;
        }

        // 否则，进入子目录继续查找
        fat_iter_init(&cur_iter, fat_fs, found_cluster);
    }

    return UINT32_MAX;
}
my_dirent* fat_open_dir(const char* block_file,
                        const char* path)
{
    FATHandle handle;
    fat_fs_mount(block_file, &handle);
    uint32_t start_clu = fat_find_start_clu(path, &handle);
    if (start_clu == UINT32_MAX)
    {
        fat_fs_unmount(&handle);
        return NULL;
    }
    else
    {
        FATIter* iter = malloc(sizeof(my_dirent));
        fat_iter_init(iter, &handle, start_clu);
        return iter;
    }
}
void fat_close_dir(my_dirent* dirent)
{
    fat_fs_unmount(&dirent->fat_fs);
    free(dirent);
}
int fat_read_dir(my_dirent* dirent, DIR* dp_out)
{
    return fat_iter_next(dirent, dp_out);
}
#ifdef _DEBUG
void test_find_base_from_name()
{
    my_dirent* h =
        fat_open_dir("./setup_bash/fat32.img", "/");
    uint32_t s = fat_find_base_path_file_clu(h, "1.txt");
    printf("%d\n", s);
    fat_close_dir(h);
}
void test_open_dir_root()
{
    my_dirent* h =
        fat_open_dir("./setup_bash/fat32.img", "/");
    DIR dp;
    fat_read_dir(h, &dp);
    printf("%.*s", dp.name_len, dp.name);
    fat_close_dir(h);
}
void test_open_dir_any()
{
    my_dirent* h =
        fat_open_dir("./setup_bash/fat32.img", "/33.dd");
    DIR dp;
    fat_read_dir(h, &dp);
    printf("%.*s", dp.name_len, dp.name);
    fat_close_dir(h);
}
#endif