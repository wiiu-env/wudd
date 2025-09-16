/****************************************************************************
 * Copyright (C) 2021 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "WriteOnlyFileWithCache.h"
#include <coreinit/memory.h>
#include <malloc.h>
#include <utils/StringTools.h>
#include <utils/logger.h>

#define SPLIT_SIZE (0x80000000)

WriteOnlyFileWithCache::WriteOnlyFileWithCache(const char *path, int32_t cacheSize, bool split) : CFile(split ? std::string(path) + ".part01" : path, WriteOnly),
                                                                                                  splitFile(split),
                                                                                                  originalPath(path) {
    if (!this->isOpen()) {
        return;
    }
    this->writeBufferSize = cacheSize;
    this->writeBuffer     = (void *) memalign(0x1000, this->writeBufferSize);
    if (this->writeBuffer == nullptr) {
        this->close();
        return;
    }
    this->writeBufferPos = 0;
}

WriteOnlyFileWithCache::~WriteOnlyFileWithCache() {
    WriteOnlyFileWithCache::close();
    free(writeBuffer);
}

bool WriteOnlyFileWithCache::flush() {
    if (this->writeBufferPos > 0) {
        int32_t res = CFile::write(static_cast<const uint8_t *>(this->writeBuffer), this->writeBufferPos);
        if (res != (int32_t) this->writeBufferPos) {
            DEBUG_FUNCTION_LINE_ERR("Failed to flush cache, write failed: %d (expected %d)", res, this->writeBufferPos);
            return false;
        }
        this->writeBufferPos = 0;
    }
    return true;
}

int32_t WriteOnlyFileWithCache::write(const uint8_t *addr, size_t writeSize) {
    auto finalAddr        = addr;
    size_t finalWriteSize = writeSize;
    if (splitFile) {
        if (pos + writeBufferPos + finalWriteSize >= (uint64_t) SPLIT_SIZE) {
            DEBUG_FUNCTION_LINE("We need to split files");
            if (!flush()) {
                return -2;
            }

            uint32_t realWriteSize = SPLIT_SIZE - pos;

            if (realWriteSize > 0) {
                DEBUG_FUNCTION_LINE_VERBOSE("Write remaining %8d bytes", realWriteSize);
                if (CFile::write(reinterpret_cast<const uint8_t *>(addr), realWriteSize) != (int32_t) realWriteSize) {
                    return -3;
                }
            }
            finalWriteSize = writeSize - realWriteSize;
            finalAddr      = (uint8_t *) ((uint32_t) addr + realWriteSize);
            part++;
            if (!flush()) {
                return -2;
            }
            CFile::close();

            // open the next part
            DEBUG_FUNCTION_LINE("Open %s", string_format("%s.part%02d", originalPath.c_str(), part).c_str());
            this->open(string_format("%s.part%02d", originalPath.c_str(), part), WriteOnly);
        }
        if (finalWriteSize == 0) {
            return (int32_t) writeSize;
        }
    }

    if (finalWriteSize == this->writeBufferSize) {
        if (!this->flush()) {
            DEBUG_FUNCTION_LINE_ERR("Flush failed");
            return -1;
        }
        return CFile::write(reinterpret_cast<const uint8_t *>(addr), finalWriteSize);
    }

    auto toWrite = (int32_t) finalWriteSize;
    if (toWrite == 0) {
        return 0;
    }

    auto written = (int32_t) (writeSize - finalWriteSize);

    do {
        int32_t curWrite = toWrite;

        if (this->writeBufferPos + curWrite > this->writeBufferSize) {
            curWrite = this->writeBufferSize - this->writeBufferPos;
        }
        OSBlockMove((void *) (((uint32_t) this->writeBuffer) + this->writeBufferPos), (void *) (finalAddr + written), curWrite, 1);
        this->writeBufferPos += curWrite;

        if (this->writeBufferPos == this->writeBufferSize) {
            if (!this->flush()) {
                DEBUG_FUNCTION_LINE_ERR("Flush failed");
                return -2;
            }
        }

        toWrite -= curWrite;
        written += curWrite;
    } while (toWrite > 0);
    return written;
}

int32_t WriteOnlyFileWithCache::seek(int64_t offset, int32_t origin) {
    // Hacky trick because we may need a seek.
    if (origin == SEEK_SET_BASE_CLASS) {
        if (splitFile) {
            if ((offset / SPLIT_SIZE) + 1 != part) {
                flush();
                close();
                part = (offset / SPLIT_SIZE) + 1;
                DEBUG_FUNCTION_LINE("Open %s", string_format("%s.part%02d", originalPath.c_str(), part).c_str());
                this->open(string_format("%s.part%02d", originalPath.c_str(), part), ReadWrite);
            }
            return CFile::seek(offset % SPLIT_SIZE, SEEK_SET);
        }
        return CFile::seek(offset, SEEK_SET);
    }
    return -1;
}

int32_t WriteOnlyFileWithCache::read(uint8_t *ptr, size_t size) {
    return -1;
}
