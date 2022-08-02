#include "common.h"
ntfs_md *ntfs_mounts              = nullptr;
int ntfs_mount_count              = 0;
BOOL gRunFromHBL                  = false;
BOOL gBlockHomeButton             = false;
uint32_t gBlockHomeButtonCooldown = 0;
FSAClientHandle gFSAClientHandle  = 0;