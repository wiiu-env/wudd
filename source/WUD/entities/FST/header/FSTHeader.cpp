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
#include "FSTHeader.h"
#include <utils/blocksize/SectionBlockSize.h>
#include <utils/logger.h>

std::optional<std::unique_ptr<FSTHeader>> FSTHeader::make_unique(const std::array<uint8_t, FSTHeader::LENGTH> &data) {
    auto *dataAsUint = (uint32_t *) data.data();
    if ((dataAsUint[0] & 0xFFFFFF00) != 0x46535400) {
        DEBUG_FUNCTION_LINE_ERR("FST Header magic was wrong");
        return {};
    }
    auto FSTVersion       = data[3];
    auto blockSize        = SectionBlockSize(dataAsUint[1]);
    auto numberOfSections = dataAsUint[2];
    auto hashDisabled     = data[12];

    return std::unique_ptr<FSTHeader>(new FSTHeader(
            FSTVersion,
            blockSize,
            numberOfSections,
            hashDisabled));
}

FSTHeader::FSTHeader(uint8_t pFSTVersion, SectionBlockSize pBlockSize, uint32_t pNumberOfSections, uint8_t pHashDisabled) : FSTVersion(pFSTVersion),
                                                                                                                            blockSize(pBlockSize),
                                                                                                                            numberOfSections(pNumberOfSections),
                                                                                                                            hashDisabled(pHashDisabled) {
}
