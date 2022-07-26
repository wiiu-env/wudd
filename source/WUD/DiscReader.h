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
#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

class DiscReader {
public:
    DiscReader();

    virtual ~DiscReader();

    virtual bool IsReady() = 0;

    virtual bool readEncryptedSector(uint8_t *buffer, uint32_t block_cnt, uint32_t block_offset) const = 0;

    bool readEncryptedAligned(uint8_t *buf, uint32_t block_offset, uint32_t size);

    bool readDecryptedChunk(uint64_t readOffset, uint8_t *out_buffer, uint8_t *key, uint8_t *IV) const;

    bool readDecrypted(uint8_t *out_buffer, uint64_t clusterOffset, uint64_t fileOffset, uint32_t size, uint8_t *key, uint8_t *IV, bool useFixedIV) const;

    virtual bool readEncrypted(uint8_t *buf, uint64_t offset, uint32_t size);

    uint8_t discKey[16]{};
    bool hasDiscKey = false;

private:
    std::mutex sector_buf_mutex;
    uint8_t *sector_buf = nullptr;
};