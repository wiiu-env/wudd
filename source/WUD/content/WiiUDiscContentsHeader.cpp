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
#include <utils/blocksize/DiscBlockSize.h>
#include <coreinit/debug.h>
#include <cstdlib>
#include "WiiUDiscContentsHeader.h"

uint32_t WiiUDiscContentsHeader::LENGTH = 2048;
uint32_t WiiUDiscContentsHeader::MAGIC = 0xCCA6E67B;

WiiUDiscContentsHeader::WiiUDiscContentsHeader(DiscReader *reader, uint32_t offset) {
    auto *buffer = (uint8_t *) malloc(LENGTH);
    if (!reader->hasDiscKey) {
        if (!reader->readEncrypted(buffer, offset, LENGTH)) {
            OSFatal("WiiUDiscContentsHeader: Failed to read encrypted");
        }
    } else {
        if (!reader->readDecrypted(buffer, offset, 0, LENGTH, reader->discKey, nullptr, true)) {
            OSFatal("WiiUDiscContentsHeader: Failed to read decrypted");
        }
    }

    if (((uint32_t *) buffer)[0] != MAGIC) {
        OSFatal("WiiUDiscContentsHeader MAGIC mismatch.");
    }
    blockSize = DiscBlockSize(((uint32_t *) buffer)[1]);
    memcpy(tocHash, &buffer[8], 20);
    numberOfPartition = ((uint32_t *) buffer)[7];
    free(buffer);
}
