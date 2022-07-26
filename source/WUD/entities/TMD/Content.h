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

#include <array>
#include <cstdint>
#include <memory>
#include <optional>

class Content {
public:
    static constexpr uint32_t LENGTH = 0x30;

    uint32_t ID;
    uint16_t index;
    uint16_t type;
    uint64_t encryptedFileSize;
    std::array<uint8_t, 0x14> hash;

    static std::optional<std::shared_ptr<Content>> make_shared(const std::array<uint8_t, 0x30> &data);

private:
    explicit Content(uint32_t pId, uint16_t pIndex, uint16_t pType, uint64_t pEncryptedFileSize, const std::array<uint8_t, 0x14> &pHash);
};
