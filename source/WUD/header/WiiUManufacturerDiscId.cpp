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
#include "WiiUManufacturerDiscId.h"
#include <coreinit/debug.h>
#include <utils/logger.h>

std::optional<std::unique_ptr<WiiUManufacturerDiscId>> WiiUManufacturerDiscId::make_unique(std::shared_ptr<DiscReader> &discReader) {
    if (!discReader->IsReady()) {
        DEBUG_FUNCTION_LINE_ERR("DiscReader is not ready");
        return {};
    }
    std::array<uint8_t, WiiUManufacturerDiscId::LENGTH> data{};

    if (!discReader->readEncrypted(data.data(), 0, WiiUManufacturerDiscId::LENGTH)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read data");
        return {};
    }
    return std::unique_ptr<WiiUManufacturerDiscId>(new WiiUManufacturerDiscId(data));
}

WiiUManufacturerDiscId::WiiUManufacturerDiscId(const std::array<uint8_t, WiiUManufacturerDiscId::LENGTH> &pData) : data(pData) {
    this->data = pData;
}