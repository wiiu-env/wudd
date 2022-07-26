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
#include "StringTable.h"
#include <coreinit/debug.h>
#include <cstring>
#include <utils/logger.h>


std::optional<std::shared_ptr<StringTable>> StringTable::make_shared(const std::vector<uint8_t> &data, uint32_t offset, uint32_t stringCount) {
    if (offset >= data.size()) {
        DEBUG_FUNCTION_LINE_ERR("Invalid offset for reading StringTable");
        return {};
    }
    auto stringTable   = std::shared_ptr<StringTable>(new StringTable());
    uint32_t curOffset = 0;
    uint32_t i;
    for (i = 0; curOffset < data.size() && i < stringCount; ++curOffset) {
        if (data[offset + curOffset] == (uint8_t) 0) {
            ++i;
        }
    }

    if (i < stringCount) {
        DEBUG_FUNCTION_LINE_ERR("StringTable is broken");
        return {};
    }

    uint32_t curLength = 0;
    for (i = 0; i < stringCount; ++i) {
        curOffset                         = offset + curLength;
        stringTable->stringMap[curLength] = std::make_shared<StringEntry>(stringTable, curLength);
        stringTable->strings[curLength]   = (char *) &data[curOffset];

        curLength += strlen((char *) &data[curOffset]) + 1;
    }

    return stringTable;
}

std::optional<std::string> StringTable::getByAddress(uint32_t address) {
    if (strings.count(address) > 0) {
        return strings[address];
    }
    return {};
}

std::optional<std::shared_ptr<StringEntry>> StringTable::getStringEntry(uint32_t address) {
    if (stringMap.count(address) > 0) {
        return stringMap[address];
    }
    return {};
}

uint32_t StringTable::getSize() {
    uint32_t capacity = 1; // root entry
    for (auto &cur : strings) {
        capacity += cur.second.length() + 1;
    }
    return capacity;
}

std::optional<std::shared_ptr<StringEntry>> StringTable::getEntry(std::string &str) {
    for (auto &cur : strings) {
        if (cur.second == str) {
            return stringMap[cur.first];
        }
    }

    return {};
}
