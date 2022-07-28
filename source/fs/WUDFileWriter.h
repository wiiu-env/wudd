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

#include "WriteOnlyFileWithCache.h"

class WUDFileWriter : public WriteOnlyFileWithCache {
public:
    WUDFileWriter(const char *string, int32_t cacheSize, int32_t sectorSize, bool split = false);

    virtual int32_t writeSector(const uint8_t *buffer, uint32_t numberOfSectors);

    virtual bool finalize();

protected:
    int32_t sectorSize;
};
