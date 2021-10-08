#include <cstring>
#include <cstddef>
#include <whb/log.h>
#include "logger.h"
#include "utils.h"
#include "TinySHA1.hpp"
#include "../fs/FSUtils.h"

// https://gist.github.com/ccbrown/9722406
void Utils::dumpHex(const void *data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    DEBUG_FUNCTION_LINE_WRITE("0x%08X (0x0000): ", data);
    for (i = 0; i < size; ++i) {
        WHBLogWritef("%02X ", ((unsigned char *) data)[i]);
        if (((unsigned char *) data)[i] >= ' ' && ((unsigned char *) data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char *) data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            WHBLogWritef(" ");
            if ((i + 1) % 16 == 0) {
                WHBLogPrintf("|  %s ", ascii);
                if (i + 1 < size) {
                    DEBUG_FUNCTION_LINE_WRITE("0x%08X (0x%04X); ", ((uint32_t) data + i + 1), i + 1);
                }
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    WHBLogWritef(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    WHBLogWritef("   ");
                }
                WHBLogPrintf("|  %s ", ascii);
            }
        }
    }
}


std::string Utils::calculateSHA1(const char *buffer, size_t size) {
    sha1::SHA1 s;
    s.processBytes(buffer, size);
    uint32_t digest[5];
    s.getDigest(digest);
    char tmp[48];
    snprintf(tmp, 45, "%08X%08X%08X%08X%08X", digest[0], digest[1], digest[2], digest[3], digest[4]);
    return tmp;
}

std::string Utils::hashFile(const std::string &path) {
    uint8_t *data = NULL;
    uint32_t size = 0;
    FSUtils::LoadFileToMem(path.c_str(), &data, &size);
    if (data == NULL) {
        return calculateSHA1(NULL, 0);
    }
    std::string result = calculateSHA1(reinterpret_cast<const char *>(data), size);
    free(data);
    return result;
}
