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
#pragma once

#include "WiiUDiscId.h"
#include "WiiUManufacturerDiscId.h"
#include <WUD/DiscReader.h>
#include <WUD/DiscReaderDiscDrive.h>
#include <WUD/content/WiiUContentsInformation.h>
#include <cstdint>
#include <memory>

class WiiUDiscHeader {
public:
    static std::optional<std::unique_ptr<WiiUDiscHeader>> make_unique(std::shared_ptr<DiscReader> &discReader);

    std::unique_ptr<WiiUManufacturerDiscId> manufacturerDiscId;
    std::unique_ptr<WiiUDiscId> discId;
    std::unique_ptr<WiiUContentsInformation> wiiUContentsInformation;

    static uint32_t LENGTH;

private:
    explicit WiiUDiscHeader(
            std::unique_ptr<WiiUManufacturerDiscId> pManufacturerDiscId,
            std::unique_ptr<WiiUDiscId> pDiscId,
            std::unique_ptr<WiiUContentsInformation> pWiiUContentsInformation);
};