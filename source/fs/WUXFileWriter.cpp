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
#include "WUXFileWriter.h"
#include "WUDDumperState.h"
#include "utils/StringTools.h"
#include <utils/logger.h>

WUXFileWriter::WUXFileWriter(const char *path, int32_t cacheSize, int32_t sectorSize, bool split) : WUDFileWriter(path, cacheSize, sectorSize, split) {
    wuxHeader_t wuxHeader      = {0};
    wuxHeader.magic0           = WUX_MAGIC_0;
    wuxHeader.magic1           = WUX_MAGIC_1;
    wuxHeader.sectorSize       = swap_uint32(this->sectorSize);
    wuxHeader.uncompressedSize = swap_uint64(WUD_FILE_SIZE);
    wuxHeader.flags            = 0;

    if (this->write((uint8_t *) &wuxHeader, sizeof(wuxHeader_t)) != sizeof(wuxHeader_t)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to write header");
        WUDFileWriter::close();
        return;
    }
    this->sectorTableStart = this->tell();
    this->totalSectorCount = WUD_FILE_SIZE / this->sectorSize;

    this->sectorIndexTable = (void *) memalign(0x40, ROUNDUP(totalSectorCount * 4, 0x40));
    if (sectorIndexTable == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc");
        WUDFileWriter::close();
        return;
    }
    memset(this->sectorIndexTable, 0, totalSectorCount * 4);

    if (this->write((uint8_t *) this->sectorIndexTable, totalSectorCount * 4) != (int32_t) ((uint32_t) totalSectorCount * 4)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to write initial sector index table");
        WUDFileWriter::close();
        return;
    }

    this->sectorTableEnd = this->tell();
    uint64_t tableEnd    = this->sectorTableEnd;

    this->sectorTableEnd += this->sectorSize - 1;
    this->sectorTableEnd -= (this->sectorTableEnd % this->sectorSize);

    uint64_t padding  = this->sectorTableEnd - tableEnd;
    auto *paddingData = (uint8_t *) memalign(0x40, ROUNDUP(padding, 0x40));
    memset(paddingData, 0, padding);
    if (this->write(reinterpret_cast<const uint8_t *>(paddingData), padding) != (int32_t) padding) {
        DEBUG_FUNCTION_LINE_ERR("Failed to write padding.");
        WUDFileWriter::close();
        return;
    }
    free(paddingData);
    this->hashMap.clear();
    if (!flush()) {
        WUXFileWriter::close();
    }
}


int32_t WUXFileWriter::writeSector(const uint8_t *buffer, uint32_t numberOfSectors) {
    if (!isOpen()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to write sector, file is not open");
        return -1;
    }
    int32_t curWrittenSectors = 0;
    for (uint32_t i = 0; i < numberOfSectors; i++) {
        uint32_t addr = ((uint32_t) buffer) + (i * this->sectorSize);
        std::array<uint8_t, 32> hashOut{};
        calculateHash256(reinterpret_cast<unsigned char *>(addr), this->sectorSize, reinterpret_cast<unsigned char *>(hashOut.data()));

        auto *indexTable = (uint32_t *) this->sectorIndexTable;

        auto it = hashMap.find(hashOut);
        if (it != hashMap.end()) {
            indexTable[this->currentSector] = swap_uint32(this->hashMap[hashOut]);
        } else {
            indexTable[this->currentSector] = swap_uint32(this->writtenSector);
            hashMap[hashOut]                = writtenSector;
            if (isOpen()) {
                if (write((uint8_t *) addr, this->sectorSize) != this->sectorSize) {
                    DEBUG_FUNCTION_LINE_ERR("Write failed");
                    return -1;
                }
            }
            this->writtenSector++;
            curWrittenSectors++;
        }
        this->currentSector++;
    }
    return curWrittenSectors;
}

bool WUXFileWriter::writeSectorIndexTable() {
    if (this->isOpen()) {
        if (!flush()) {
            return false;
        }
        // We need to make sure to call CFile::seek!
        if (seek((int64_t) sectorTableStart, SEEK_SET_BASE_CLASS) < 0) {
            DEBUG_FUNCTION_LINE_ERR("Seek failed");
            return false;
        }
        if (write((uint8_t *) sectorIndexTable, totalSectorCount * 4) != (int32_t) ((uint32_t) totalSectorCount * 4)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to write sector index table");
            return false;
        }
        if (!flush()) {
            return false;
        }
    }
    return true;
}

WUXFileWriter::~WUXFileWriter() {
    WUXFileWriter::flush();
    WUXFileWriter::close();
    free(sectorIndexTable);
}

bool WUXFileWriter::finalize() {
    WUDFileWriter::finalize();
    bool res = writeSectorIndexTable();
    WUXFileWriter::close();
    return res;
}
