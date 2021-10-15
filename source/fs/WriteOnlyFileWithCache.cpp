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
#include <malloc.h>
#include <coreinit/memory.h>
#include <utils/logger.h>
#include "WriteOnlyFileWithCache.h"

WriteOnlyFileWithCache::WriteOnlyFileWithCache(const char *path, CFile::eOpenTypes mode, int32_t cacheSize) : CFile(path, mode) {
    if (!this->isOpen()) {
        return;
    }
    this->writeBufferSize = cacheSize;
    this->writeBuffer = (void *) memalign(0x1000, this->writeBufferSize);
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
        if (res < 0) {
            DEBUG_FUNCTION_LINE("Failed to flush cache, write failed: %d", res);
            return false;
        }
        this->writeBufferPos = 0;
    }
    return true;
}

int32_t WriteOnlyFileWithCache::write(const uint8_t *addr, size_t writeSize) {
    if (writeSize == this->writeBufferSize) {
        if (!this->flush()) {
            DEBUG_FUNCTION_LINE("Flush failed");
            return -1;
        }
        return CFile::write(reinterpret_cast<const uint8_t *>(addr), writeSize);
    }

    auto toWrite = (int32_t) writeSize;
    if (toWrite == 0) {
        return 0;
    }

    int32_t written = 0;

    do {
        int32_t curWrite = toWrite;

        if (this->writeBufferPos + curWrite > this->writeBufferSize) {
            curWrite = this->writeBufferSize - this->writeBufferPos;
        }
        OSBlockMove((void *) (((uint32_t) this->writeBuffer) + this->writeBufferPos), (void *) (addr + written), curWrite, 1);
        this->writeBufferPos += curWrite;

        if (this->writeBufferPos == this->writeBufferSize) {
            if (!this->flush()) {
                DEBUG_FUNCTION_LINE("Flush failed");
                return -2;
            }
        }

        toWrite -= curWrite;
        written += curWrite;
    } while (toWrite > 0);
    return written;
}

int32_t WriteOnlyFileWithCache::seek(int64_t offset, int32_t origin) {
    return -1;
}

int32_t WriteOnlyFileWithCache::read(uint8_t *ptr, size_t size) {
    return -1;
}
