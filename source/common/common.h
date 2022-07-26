#pragma once

#include <ntfs.h>
#include <wut.h>

#define SECTOR_SIZE      0x8000
#define READ_SECTOR_SIZE SECTOR_SIZE

extern ntfs_md *ntfs_mounts;
extern int ntfs_mount_count;

extern "C" FSClient *__wut_devoptab_fs_client;

extern BOOL gRunFromHBL;
extern BOOL gBlockHomeButton;
extern uint32_t gBlockHomeButtonCooldown;

enum eDumpTarget {
    TARGET_SD,
    TARGET_NTFS
};