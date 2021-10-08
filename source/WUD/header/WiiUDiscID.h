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

#include <string>
#include <WUD/DiscReader.h>

class WiiUDiscID {

public:
    WiiUDiscID(DiscReader *reader, uint32_t offset);

    static uint32_t LENGTH;
    static uint32_t MAGIC;
    uint8_t majorVersion;
    uint8_t minorVersion;
    std::string footprint;
};
