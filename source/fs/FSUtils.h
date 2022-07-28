#pragma once

#include <string>
#include <wut_types.h>

class FSUtils {
public:
    static int32_t CreateSubfolder(const char *fullpath);

    static int32_t CheckFile(const char *filepath);

    static int32_t saveBufferToFile(const char *path, void *buffer, uint32_t size);
};