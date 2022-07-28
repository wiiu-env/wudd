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
#include "WiiUDiscContentsHeader.h"
#include "utils/utils.h"
#include <coreinit/debug.h>
#include <utils/blocksize/DiscBlockSize.h>

std::optional<std::unique_ptr<WiiUDiscContentsHeader>> WiiUDiscContentsHeader::make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset) {
    auto buffer = make_unique_nothrow<uint8_t[]>(LENGTH);
    if (!buffer) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc buffer");
        return {};
    }
    if (!discReader->hasDiscKey) {
        if (!discReader->readEncrypted(buffer.get(), offset, LENGTH)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to read data");
            return {};
        }
    } else {
        if (!discReader->readDecrypted(buffer.get(), offset, 0, LENGTH, discReader->discKey, nullptr, true)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to read data");
            return {};
        }
    }

    if (((uint32_t *) buffer.get())[0] != MAGIC) {
        DEBUG_FUNCTION_LINE_ERR("MAGIC mismatch");
        return {};
    }
    auto blockSize = DiscBlockSize(((uint32_t *) buffer.get())[1]);
    std::array<uint8_t, 20> tocHash{};
    memcpy(tocHash.data(), &buffer[8], 20);
    auto numberOfPartition = ((uint32_t *) buffer.get())[7];

    return std::unique_ptr<WiiUDiscContentsHeader>(new WiiUDiscContentsHeader(blockSize, tocHash, numberOfPartition));
}

WiiUDiscContentsHeader::WiiUDiscContentsHeader(DiscBlockSize pSize, const std::array<uint8_t, 20> &pTocHash, uint32_t pNumberOfPartitions) : blockSize(pSize),
                                                                                                                                             numberOfPartition(pNumberOfPartitions),
                                                                                                                                             tocHash(pTocHash) {
}
