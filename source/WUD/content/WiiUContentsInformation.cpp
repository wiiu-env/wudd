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
#include <coreinit/debug.h>
#include "WiiUContentsInformation.h"

uint32_t  WiiUContentsInformation::LENGTH = 32768;

WiiUContentsInformation::WiiUContentsInformation(DiscReader *reader, uint32_t offset) {
    uint32_t curOffset = offset;
    discContentHeader = new WiiUDiscContentsHeader(reader, curOffset);
    curOffset += WiiUDiscContentsHeader::LENGTH;

    partitions = new WiiUPartitions(reader, curOffset, discContentHeader->numberOfPartition, discContentHeader->blockSize);
    curOffset += WiiUPartitions::LENGTH;

    if (curOffset - offset != LENGTH) {
        OSFatal("Length mismatch");
    }
}

WiiUContentsInformation::~WiiUContentsInformation() {
    delete partitions;
    delete discContentHeader;
}
