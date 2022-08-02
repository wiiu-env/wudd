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
#include "DiscReaderDiscDrive.h"
#include "utils/utils.h"
#include <WUD/content/WiiUDiscContentsHeader.h>
#include <common/common.h>
#include <malloc.h>
#include <mocha/fsa.h>
#include <mocha/mocha.h>
#include <utils/logger.h>
#include <utils/rijndael.h>

DiscReaderDiscDrive::DiscReaderDiscDrive() : DiscReader() {
    auto sector_buf = (uint8_t *) memalign(0x40, (size_t) READ_SECTOR_SIZE);
    if (!sector_buf) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate buffer");
        return;
    }

    auto ret = FSAEx_RawOpenEx(gFSAClientHandle, "/dev/odd01", &device_handle);
    if (ret < 0) {
        free(sector_buf);
        return;
    }

    auto res = FSAEx_RawReadEx(gFSAClientHandle, sector_buf, READ_SECTOR_SIZE, 1, 3, device_handle);
    if (res >= 0) {
        if (((uint32_t *) sector_buf)[0] != WiiUDiscContentsHeader::MAGIC) {
            uint8_t iv[16];
            memset(iv, 0, 16);

            WUDDiscKey discKeyLocal;

            auto discKeyRes = Mocha_ODMGetDiscKey(&discKeyLocal);
            if (discKeyRes == MOCHA_RESULT_SUCCESS) {
                hasDiscKey = true;
                memcpy(this->discKey, discKeyLocal.key, 16);
                auto sector_buf_decrypted = make_unique_nothrow<uint8_t[]>((size_t) READ_SECTOR_SIZE);
                if (!sector_buf_decrypted) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to allocate memory");
                    free(sector_buf);
                    return;
                }
                aes_set_key((uint8_t *) discKey);
                aes_decrypt((uint8_t *) iv, (uint8_t *) &sector_buf[0], (uint8_t *) &sector_buf_decrypted[0], READ_SECTOR_SIZE);
                if (((uint32_t *) sector_buf_decrypted.get())[0] == WiiUDiscContentsHeader::MAGIC) {
                    this->init_done = true;
                } else {
                    DEBUG_FUNCTION_LINE_ERR("Invalid disc key");
                }
            } else {
                DEBUG_FUNCTION_LINE_ERR("Failed to get the DiscKey");
                this->init_done = false;
            }
        } else {
            this->init_done = true;
        }
    } else {
        DEBUG_FUNCTION_LINE_ERR("Raw read failed %d", ret);
    }
}

bool DiscReaderDiscDrive::readEncryptedSector(uint8_t *buffer, uint32_t block_cnt, uint32_t block_offset) const {
    if (FSAEx_RawReadEx(gFSAClientHandle, buffer, READ_SECTOR_SIZE, block_cnt, block_offset, device_handle) < 0) {
        return false;
    }
    return true;
}

bool DiscReaderDiscDrive::IsReady() {
    return init_done;
}

DiscReaderDiscDrive::~DiscReaderDiscDrive() {
    if (device_handle != -1) {
        FSAEx_RawCloseEx(gFSAClientHandle, device_handle);
        device_handle = -1;
    }
}

bool DiscReaderDiscDrive::readEncrypted(uint8_t *buf, uint64_t offset, uint32_t size) {
    if (size == 0) {
        return true;
    }

    if ((offset & (SECTOR_SIZE - 0x1)) != 0 || (size & (SECTOR_SIZE - 0x1)) != 0) {
        return DiscReader::readEncrypted(buf, offset, size);
    }
    uint32_t block_cnt         = size >> 15;
    uint32_t offset_in_sectors = offset >> 15;
    if (FSAEx_RawReadEx(gFSAClientHandle, buf, 0x8000, block_cnt, offset_in_sectors, device_handle) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read from Disc");
        return false;
    }
    return true;
}

std::optional<std::unique_ptr<DiscReaderDiscDrive>> DiscReaderDiscDrive::make_unique() {
    auto discReader = make_unique_nothrow<DiscReaderDiscDrive>();
    if (!discReader || !discReader->IsReady()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init DiscReader %d %d", !discReader, discReader->IsReady());
        return {};
    }
    return discReader;
}
