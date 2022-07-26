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
#include "H3HashArray.h"
#include "utils/utils.h"
#include <coreinit/debug.h>
#include <cstring>

H3HashArray::H3HashArray(uint8_t *pData, uint32_t pSize) {
    size = pSize;
    data = nullptr;
    if (pSize > 0) {
        data = make_unique_nothrow<uint8_t[]>(pSize);
        if (!data) {
            OSFatal("H3HashArray: Failed to alloc");
        }
        memcpy(data.get(), pData, pSize);
    }
}

H3HashArray::~H3HashArray() = default;