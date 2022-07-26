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

#include "StringEntry.h"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class StringTable {

public:
    static std::optional<std::shared_ptr<StringTable>> make_shared(const std::vector<uint8_t> &data, uint32_t offset, uint32_t stringCount);

    std::optional<std::string> getByAddress(uint32_t address);

    std::optional<std::shared_ptr<StringEntry>> getStringEntry(uint32_t address);

    uint32_t getSize();

    std::optional<std::shared_ptr<StringEntry>> getEntry(std::string &str);

private:
    StringTable() = default;

    std::map<uint32_t, std::shared_ptr<StringEntry>> stringMap;
    std::map<uint32_t, std::string> strings;
};