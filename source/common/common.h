#pragma once

#include <ntfs.h>
#include <wut.h>

extern int32_t gFSAfd;

#define SECTOR_SIZE      0x8000
#define READ_SECTOR_SIZE SECTOR_SIZE

extern ntfs_md *ntfs_mounts;
extern int ntfs_mount_count;

extern BOOL gRunFromHBL;

enum eDumpTarget {
    TARGET_SD,
    TARGET_NTFS
};