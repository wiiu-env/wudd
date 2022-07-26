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
#include "utils/utils.h"
#include <coreinit/debug.h>
#include <memory>
#include <utility>
#include <utils/logger.h>

std::optional<std::unique_ptr<WiiUDiscId>> WiiUDiscId::make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset) {
    auto data = make_unique_nothrow<uint8_t[]>(WiiUDiscId::LENGTH);
    if (data == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc memory");
        return {};
    }

    if (!discReader->readEncrypted(data.get(), offset, WiiUDiscId::LENGTH)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read data");
        return {};
    }

    if (((uint32_t *) data.get())[0] != WiiUDiscId::MAGIC) {
        DEBUG_FUNCTION_LINE_ERR("MAGIC mismatch");
        return {};
    }

    auto majorVersion = data[5];
    auto minorVersion = data[6];

    auto footprint = std::string((char *) &data[32]);

    return std::unique_ptr<WiiUDiscId>(new WiiUDiscId(minorVersion, majorVersion, footprint));
}

WiiUDiscId::WiiUDiscId(uint8_t pMinorVersion, uint8_t pMajorVersion, std::string pFootprint) : minorVersion(pMinorVersion),
                                                                                               majorVersion(pMajorVersion),
                                                                                               footprint(std::move(pFootprint)) {
}
