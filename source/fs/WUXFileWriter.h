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
#pragma once

#include "WUDFileWriter.h"
#include "utils/utils.h"
#include <map>

typedef struct {
    unsigned int magic0;
    unsigned int magic1;
    unsigned int sectorSize;
    unsigned long long uncompressedSize;
    unsigned int flags;
} wuxHeader_t;

#define WUX_MAGIC_0 0x57555830
#define WUX_MAGIC_1 swap_uint32(0x1099d02e)

class WUXFileWriter : public WUDFileWriter {
public:
    WUXFileWriter(const char *string, int32_t cacheSize, int32_t pSectorSize, bool split = false);

    ~WUXFileWriter() override;

    int32_t writeSector(const uint8_t *buffer, uint32_t numberOfSectors) override;

    bool finalize() override;

private:
    bool writeSectorIndexTable();

    uint64_t totalSectorCount;

    uint64_t sectorTableStart;
    uint64_t sectorTableEnd;
    void *sectorIndexTable = nullptr;

    std::map<std::array<uint8_t, 32>, uint32_t> hashMap;
    uint32_t currentSector = 0;
    uint32_t writtenSector = 0;
};