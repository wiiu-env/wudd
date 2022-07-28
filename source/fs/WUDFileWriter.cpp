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
#include "WUDFileWriter.h"
#include <utils/logger.h>

WUDFileWriter::WUDFileWriter(const char *path, int32_t cacheSize, int32_t pSectorSize, bool split) : WriteOnlyFileWithCache(path, cacheSize, split),
                                                                                                     sectorSize(pSectorSize) {
}

int32_t WUDFileWriter::writeSector(const uint8_t *buffer, uint32_t numberOfSectors) {
    auto result = write(buffer, numberOfSectors * this->sectorSize);
    if (result == (int32_t) (numberOfSectors * this->sectorSize)) {
        return (int32_t) numberOfSectors;
    }
    return -1;
}

bool WUDFileWriter::finalize() {
    return true;
}
