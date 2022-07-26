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

#include "BlockSize.h"
#include <iostream>
#include <type_traits>

class SizeInBlocks {
public:
    SizeInBlocks() = default;

    explicit SizeInBlocks(const BlockSize &blockSize, uint32_t pValue) : blockSize(blockSize) {
        value = pValue;
    }

    virtual uint64_t getSizeInBytes() {
        return (uint64_t) value * blockSize.blockSize;
    }

    BlockSize blockSize;
    uint64_t value{};
};