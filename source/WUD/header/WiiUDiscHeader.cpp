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
#include "WiiUDiscHeader.h"
#include <coreinit/debug.h>

uint32_t WiiUDiscHeader::LENGTH = 131072L;

WiiUDiscHeader::WiiUDiscHeader(DiscReader *reader, uint32_t offset) {
    uint32_t curOffset = offset;
    manufactorDiscID = new WiiUManufactorDiscID(reader, 0);
    curOffset += WiiUManufactorDiscID::LENGTH;
    discId = new WiiUDiscID(reader, curOffset);
    curOffset += WiiUDiscID::LENGTH;
    wiiUContentsInformation = new WiiUContentsInformation(reader, curOffset);
    curOffset += WiiUContentsInformation::LENGTH;

    if (curOffset - offset != LENGTH) {
        OSFatal("Length mismatch");
    }
}

WiiUDiscHeader::~WiiUDiscHeader() {
    delete manufactorDiscID;
    delete discId;
    delete wiiUContentsInformation;
}
