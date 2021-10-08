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
#include "WiiUDiscID.h"
#include <coreinit/debug.h>

uint32_t WiiUDiscID::LENGTH = 32768;
uint32_t WiiUDiscID::MAGIC = 0xCC549EB9;

WiiUDiscID::WiiUDiscID(DiscReader *reader, uint32_t offset) {
    auto data = (uint8_t *) malloc(LENGTH);
    if (data == nullptr) {
        OSFatal("Failed to alloc for WiiUDiscID");
    }

    if (!reader->readEncrypted(data, offset, LENGTH)) {
        OSFatal("Failed to read data");
    }

    if (((uint32_t *) data)[0] != MAGIC) {
        OSFatal("MAGIC FAIL");
    }

    majorVersion = data[5];
    minorVersion = data[6];

    footprint = std::string((char *) &data[32]);

    free(data);
}
