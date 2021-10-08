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
#include <utils/blocksize/SectionBlockSize.h>
#include <coreinit/debug.h>
#include "FSTHeader.h"

FSTHeader::FSTHeader(uint8_t *data) {
    auto *dataAsUint = (uint32_t *) data;
    if ((dataAsUint[0] & 0xFFFFFF00) != 0x46535400) {
        OSFatal("FST Header magic was wrong");
    }
    FSTVersion = data[3];
    blockSize = SectionBlockSize(dataAsUint[1]);
    numberOfSections = dataAsUint[2];
    hashDisabled = data[12];
}
