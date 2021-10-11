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
#include <WUD/content/WiiUDiscContentsHeader.h>
#include <common/common.h>
#include <ApplicationState.h>
#include <iosuhax.h>
#include <utils/rijndael.h>
#include <utils/logger.h>
#include "DiscReaderDiscDrive.h"


DiscReaderDiscDrive::DiscReaderDiscDrive() : DiscReader() {
    DEBUG_FUNCTION_LINE();
    auto *sector_buf = (uint8_t *) malloc(READ_SECTOR_SIZE);
    if (sector_buf == nullptr) {
        return;
    }

    auto ret = IOSUHAX_FSA_RawOpen(gFSAfd, "/dev/odd01", &device_handle);
    if (ret < 0) {
        return;
    }

    auto res = IOSUHAX_FSA_RawRead(gFSAfd, sector_buf, READ_SECTOR_SIZE, 1, 3, device_handle);

    if (res >= 0) {
        if (((uint32_t *) sector_buf)[0] != WiiUDiscContentsHeader::MAGIC) {
            uint8_t iv[16];
            memset(iv, 0, 16);

            auto discKeyRes = IOSUHAX_ODM_GetDiscKey(discKey);
            if (discKeyRes >= 0) {
                hasDiscKey = true;
                auto sector_buf_decrypted = (uint8_t *) malloc(READ_SECTOR_SIZE);
                if (sector_buf_decrypted != nullptr) {
                    aes_set_key((uint8_t *) discKey);
                    aes_decrypt((uint8_t *) iv, (uint8_t *) sector_buf, (uint8_t *) &sector_buf_decrypted[0], READ_SECTOR_SIZE);
                    if (((uint32_t *) sector_buf_decrypted)[0] == WiiUDiscContentsHeader::MAGIC) {
                        DEBUG_FUNCTION_LINE("Key was correct");
                        this->init_done = true;
                    }
                    free(sector_buf_decrypted);
                }
            }
        } else {
            this->init_done = true;
        }
    }
    free(sector_buf);
}

bool DiscReaderDiscDrive::readEncryptedSector(uint8_t *buffer, uint32_t block_cnt, uint64_t offset_in_sectors) const {
    if (IOSUHAX_FSA_RawRead(gFSAfd, buffer, READ_SECTOR_SIZE, block_cnt, offset_in_sectors, device_handle) < 0) {
        DEBUG_FUNCTION_LINE("Failed to read from Disc");
        return false;
    }
    return true;
}

bool DiscReaderDiscDrive::IsReady() {
    return init_done;
}

DiscReaderDiscDrive::~DiscReaderDiscDrive() {
    DEBUG_FUNCTION_LINE();
    if (device_handle != -1) {
        IOSUHAX_FSA_RawOpen(gFSAfd, "/dev/odd01", &device_handle);
    }
}

bool DiscReaderDiscDrive::readEncrypted(uint8_t *buf, uint64_t offset, uint32_t size) {
    if (size == 0) {
        return true;
    }
    if ((offset & 0x7FFF) != 0 || (size & 0x7FFF) != 0) {
        return DiscReader::readEncrypted(buf, offset, size);
    }
    uint32_t block_cnt = size >> 15;
    uint32_t offset_in_sectors = offset >> 15;
    if (IOSUHAX_FSA_RawRead(gFSAfd, buf, 0x8000, block_cnt, offset_in_sectors, device_handle) < 0) {
        DEBUG_FUNCTION_LINE("Failed to read from Disc");
        return false;
    }
    return true;
}

std::optional<DiscReaderDiscDrive *> DiscReaderDiscDrive::Create() {
    auto discReader = new DiscReaderDiscDrive();
    if (!discReader->IsReady()) {
        delete discReader;
        return {};
    }
    return discReader;
}
