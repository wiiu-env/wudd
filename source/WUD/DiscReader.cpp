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

#include <MainApplicationState.h>
#include <WUD/content/WiiUDiscContentsHeader.h>
#include <common/common.h>
#include <coreinit/debug.h>
#include <utils/rijndael.h>

bool DiscReader::readDecryptedChunk(uint64_t readOffset, uint8_t *out_buffer, uint8_t *key, uint8_t *IV) const {
    int CHUNK_SIZE = 0x10000;

    uint32_t sectorOffset = readOffset / READ_SECTOR_SIZE;

    auto *encryptedBuffer = (uint8_t *) malloc(CHUNK_SIZE);

    if (encryptedBuffer == nullptr) {
        DEBUG_FUNCTION_LINE("Failed to alloc buffer");
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
    auto *buffer            = (uint8_t *) malloc(BLOCK_SIZE);
    if (buffer == nullptr) {
        return false;
    }

    uint32_t maxCopySize;
    uint32_t copySize;

    uint64_t readOffset;

    uint32_t totalread = 0;

    bool result = true;

    do {
        uint64_t totalOffset = (clusterOffset + usedFileOffset);
        uint64_t blockNumber = (totalOffset / BLOCK_SIZE);
        uint64_t blockOffset = (totalOffset % BLOCK_SIZE);

        readOffset = (blockNumber * BLOCK_SIZE);
        if (!useFixedIV) {
            memset(usedIV, 0, 16);
            uint64_t ivTemp = usedFileOffset >> 16;
            memcpy(usedIV + 8, &ivTemp, 8);
        }

        if (!readDecryptedChunk(readOffset, buffer, key, usedIV)) {
            result = false;
            break;
        }
        maxCopySize = BLOCK_SIZE - blockOffset;
        copySize    = (usedSize > maxCopySize) ? maxCopySize : usedSize;

        memcpy(out_buffer + totalread, buffer + blockOffset, copySize);

        totalread += copySize;

        // update counters
        usedSize -= copySize;
        usedFileOffset += copySize;
    } while (totalread < size);

    free(buffer);

    return result;
}

bool DiscReader::readEncryptedAligned(uint8_t *buf, uint64_t offset_in_sector, uint32_t size) {
    auto full_block_count = size / SECTOR_SIZE;
    if (full_block_count > 0) {
        if (!readEncryptedSector(buf, full_block_count, offset_in_sector)) {
            return false;
        }
    }

    auto remainingSize = size - (full_block_count * SECTOR_SIZE);
    if (remainingSize > 0) {
        auto newOffset = offset_in_sector + full_block_count;
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
        curOffset += missingFromPrevSector;
        offsetInBuf += missingFromPrevSector;
    }

    if (totalRead >= size) {
        return true;
    }

    if (curOffset % SECTOR_SIZE == 0) {
        if (!readEncryptedAligned(buf + offsetInBuf, offset / SECTOR_SIZE, size)) {
            return false;
        }
    } else {
        OSFatal("Failed to read encrypted");
    }

    return true;
}

DiscReader::DiscReader() {
    this->sector_buf = (uint8_t *) malloc(READ_SECTOR_SIZE);
}

DiscReader::~DiscReader() {
    free(this->sector_buf);
}
