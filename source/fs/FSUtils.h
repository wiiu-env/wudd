#pragma once

#include <wut_types.h>
#include <string>

class FSUtils {
public:
    static int32_t LoadFileToMem(const char *filepath, uint8_t **inbuffer, uint32_t *size);

    static int32_t CreateSubfolder(const char *fullpath);

    static int32_t CheckFile(const char *filepath);

    static bool copyFile(const std::string &in, const std::string &out);

    static int32_t saveBufferToFile(const char *path, void *buffer, uint32_t size);
};