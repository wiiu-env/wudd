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
#include "WiiUDiscId.h"
#include <memory>
#include <coreinit/debug.h>
#include <utils/logger.h>

std::optional<std::unique_ptr<WiiUDiscId>> WiiUDiscId::make_unique(const std::shared_ptr<DiscReader> &discReader, uint32_t offset) {
    auto data = (uint8_t *) malloc(WiiUDiscId::LENGTH);
    if (data == nullptr) {
        DEBUG_FUNCTION_LINE("Failed to alloc memory");
        return {};
    }

    if (!discReader->readEncrypted(data, offset, WiiUDiscId::LENGTH)) {
        DEBUG_FUNCTION_LINE("Failed to read data");
        return {};
    }

    if (((uint32_t *) data)[0] != WiiUDiscId::MAGIC) {
        DEBUG_FUNCTION_LINE("MAGIC mismatch");
        return {};
    }

    auto majorVersion = data[5];
    auto minorVersion = data[6];

    auto footprint = std::string((char *) &data[32]);

    free(data);

    return std::unique_ptr<WiiUDiscId>(new WiiUDiscId(minorVersion, majorVersion, footprint));
}

WiiUDiscId::WiiUDiscId(uint8_t pMinorVersion, uint8_t pMajorVersion, const std::string &pFootprint) :
        minorVersion(pMinorVersion),
        majorVersion(pMajorVersion),
        footprint(pFootprint) {

}
