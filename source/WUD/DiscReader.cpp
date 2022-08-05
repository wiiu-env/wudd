/****************************************************************************
 * Copyright (C) 2016-2021 Maschell
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

#include "utils/utils.h"
#include <WUD/content/WiiUDiscContentsHeader.h>
#include <common/common.h>
#include <coreinit/debug.h>
#include <utils/rijndael.h>

bool DiscReader::readDecryptedChunk(uint64_t readOffset, uint8_t *out_buffer, uint8_t *key, uint8_t *IV) const {
    uint32_t CHUNK_SIZE = 0x10000;

    uint32_t sectorOffset = readOffset / READ_SECTOR_SIZE;

    auto encryptedBuffer = (uint8_t *) memalign(0x40, CHUNK_SIZE);

    if (!encryptedBuffer) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc buffer");
        return false;
    }

    bool result = false;
    if (readEncryptedSector(encryptedBuffer, 2, sectorOffset)) {
        aes_set_key((uint8_t *) key);
        aes_decrypt((uint8_t *) IV, (uint8_t *) encryptedBuffer, out_buffer, CHUNK_SIZE);
        // Update IV
        memcpy(IV, &encryptedBuffer[CHUNK_SIZE - 16], 16);
        result = true;
    }
    free(encryptedBuffer);

    return result;
}

bool DiscReader::readDecrypted(uint8_t *out_buffer, uint64_t clusterOffset, uint64_t fileOffset, uint32_t size, uint8_t *key, uint8_t *IV, bool useFixedIV) const {
    uint8_t backupIV[0x10];
    memset(backupIV, 0, 16);
    uint8_t *usedIV = backupIV;
    if (useFixedIV) {
        usedIV = IV;
        if (IV == nullptr) {
            usedIV = backupIV;
        }
    }

    int BLOCK_SIZE = 0x10000;

    uint32_t usedSize       = size;
    uint64_t usedFileOffset = fileOffset;
    auto buffer             = make_unique_nothrow<uint8_t[]>((size_t) BLOCK_SIZE);
    if (!buffer) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate buffer");
        return false;
    }

    uint32_t maxCopySize;
    uint32_t copySize;

    uint64_t readOffset;

    uint32_t totalread = 0;

    bool result = true;

    do {
        uint64_t totalOffset = (clusterOffset + usedFileOffset);
        uint32_t blockNumber = (totalOffset / BLOCK_SIZE);
        uint32_t blockOffset = (totalOffset % BLOCK_SIZE);

        readOffset = ((uint64_t) blockNumber * BLOCK_SIZE);
        if (!useFixedIV) {
            memset(usedIV, 0, 16);
            uint64_t ivTemp = usedFileOffset >> 16;
            memcpy(usedIV + 8, &ivTemp, 8);
        }

        if (!readDecryptedChunk(readOffset, buffer.get(), key, usedIV)) {
            result = false;
            break;
        }
        maxCopySize = BLOCK_SIZE - blockOffset;
        copySize    = (usedSize > maxCopySize) ? maxCopySize : usedSize;

        memcpy(out_buffer + totalread, buffer.get() + blockOffset, copySize);

        totalread += copySize;

        // update counters
        usedSize -= copySize;
        usedFileOffset += copySize;
    } while (totalread < size);

    return result;
}

bool DiscReader::readEncryptedAligned(uint8_t *buf, uint32_t block_offset, uint32_t size) {
    auto full_block_count = size / SECTOR_SIZE;
    if (full_block_count > 0) {
        if (!readEncryptedSector(buf, full_block_count, block_offset)) {
            return false;
        }
    }
    auto remainingSize = size - (full_block_count * SECTOR_SIZE);
    if (remainingSize > 0) {
        std::lock_guard<std::mutex> lock(sector_buf_mutex);
        auto newOffset = block_offset + full_block_count;

        if (!readEncryptedSector(sector_buf, 1, newOffset)) {
            return false;
        }
        memcpy(buf + (full_block_count * SECTOR_SIZE), sector_buf, remainingSize);
    }

    return true;
}

bool DiscReader::readEncrypted(uint8_t *buf, uint64_t offset, uint32_t size) {
    if (size == 0) {
        return true;
    }
    uint32_t missingFromPrevSector = offset % SECTOR_SIZE;
    auto curOffset                 = offset;
    uint32_t offsetInBuf           = 0;
    uint32_t totalRead             = 0;

    if (missingFromPrevSector > 0) {
        std::lock_guard<std::mutex> lock(sector_buf_mutex);
        auto offset_in_sectors = offset / SECTOR_SIZE;

        if (!readEncryptedSector(sector_buf, 1, offset_in_sectors)) {
            return false;
        }

        uint32_t toCopy = SECTOR_SIZE - missingFromPrevSector;
        if (toCopy > size) {
            toCopy = size;
        }
        memcpy(buf, sector_buf + missingFromPrevSector, toCopy);
        totalRead += toCopy;
        curOffset += toCopy;
        offsetInBuf += toCopy;
    }
    if (totalRead >= size) {
        return true;
    }

    if (curOffset % SECTOR_SIZE == 0) {
        if (!readEncryptedAligned(buf + offsetInBuf, curOffset / SECTOR_SIZE, size)) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

DiscReader::DiscReader() {
    this->sector_buf = static_cast<uint8_t *>(memalign(0x40, READ_SECTOR_SIZE));
    if (!this->sector_buf) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate sector_buf");
        OSFatal("Failed to allocate sector_buf");
    }
}

DiscReader::~DiscReader() {
    free(this->sector_buf);
    this->sector_buf = nullptr;
}
