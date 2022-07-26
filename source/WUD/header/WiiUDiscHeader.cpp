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

WiiUDiscHeader::WiiUDiscHeader(std::unique_ptr<WiiUManufacturerDiscId> pManufacturerDiscId,
                               std::unique_ptr<WiiUDiscId> pDiscId,
                               std::unique_ptr<WiiUContentsInformation> pWiiUContentsInformation) : manufacturerDiscId(std::move(pManufacturerDiscId)),
                                                                                                    discId(std::move(pDiscId)),
                                                                                                    wiiUContentsInformation(std::move(pWiiUContentsInformation)) {
}

std::optional<std::unique_ptr<WiiUDiscHeader>> WiiUDiscHeader::make_unique(std::shared_ptr<DiscReader> &discReader) {
    if (!discReader->IsReady()) {
        DEBUG_FUNCTION_LINE_ERR("DiscReader is not ready");
        return {};
    }
    uint32_t offset          = 0;
    uint32_t curOffset       = offset;
    auto manufactorDiscIDOpt = WiiUManufacturerDiscId::make_unique(discReader);
    if (!manufactorDiscIDOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read ManufactorDiscId");
        return {};
    }

    curOffset += WiiUManufacturerDiscId::LENGTH;
    auto discIdOpt = WiiUDiscId::make_unique(discReader, curOffset);
    if (!discIdOpt) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read DiscId");
        return {};
    }
    curOffset += WiiUDiscId::LENGTH;
    auto wiiUContentsInformationOpt = WiiUContentsInformation::make_unique(discReader, curOffset);
    if (!wiiUContentsInformationOpt) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read WiiUContentsInformation");
        return {};
    }
    curOffset += WiiUContentsInformation::LENGTH;

    if (curOffset - offset != LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Unexpected offset");
        return {};
    }
    return std::unique_ptr<WiiUDiscHeader>(new WiiUDiscHeader(
            std::move(manufactorDiscIDOpt.value()),
            std::move(discIdOpt.value()),
            std::move(wiiUContentsInformationOpt.value())));
}
