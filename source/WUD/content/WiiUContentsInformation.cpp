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
#include "WiiUContentsInformation.h"
#include <coreinit/debug.h>

uint32_t WiiUContentsInformation::LENGTH = 32768;

std::optional<std::unique_ptr<WiiUContentsInformation>> WiiUContentsInformation::make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset) {
    uint32_t curOffset        = offset;
    auto discContentHeaderOpt = WiiUDiscContentsHeader::make_unique(discReader, curOffset);
    if (!discContentHeaderOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read WiiUDiscContentsHeader");
        return {};
    }
    curOffset += WiiUDiscContentsHeader::LENGTH;

    auto partitionsOpt = WiiUPartitions::make_unique(discReader, curOffset, discContentHeaderOpt.value()->numberOfPartition, discContentHeaderOpt.value()->blockSize);
    if (!partitionsOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read Partitions");
        return {};
    }
    curOffset += WiiUPartitions::LENGTH;

    if (curOffset - offset != LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Unexpected offset");
        return {};
    }

    return std::unique_ptr<WiiUContentsInformation>(new WiiUContentsInformation(
            std::move(discContentHeaderOpt.value()),
            std::move(partitionsOpt.value())));
}

WiiUContentsInformation::WiiUContentsInformation(std::unique_ptr<WiiUDiscContentsHeader> pDiscContentHeader,
                                                 std::unique_ptr<WiiUPartitions> pPartitions) : discContentHeader(std::move(pDiscContentHeader)),
                                                                                                partitions(std::move(pPartitions)) {
}
