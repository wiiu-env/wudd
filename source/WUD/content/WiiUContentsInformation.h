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

#include "WiiUDiscContentsHeader.h"
#include "partitions/WiiUPartitions.h"
#include <WUD/DiscReader.h>
#include <memory>

class WiiUContentsInformation {

public:
    std::unique_ptr<WiiUDiscContentsHeader> discContentHeader;

    std::unique_ptr<WiiUPartitions> partitions;

    static uint32_t LENGTH;

    static std::optional<std::unique_ptr<WiiUContentsInformation>> make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset);

private:
    WiiUContentsInformation(std::unique_ptr<WiiUDiscContentsHeader> pDiscContentsHeader, std::unique_ptr<WiiUPartitions> pPartitions);
};
