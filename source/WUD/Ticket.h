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
#include <cstring>
#include <memory>
#include <optional>
#include <utils/rijndael.h>
#include <vector>

class Ticket {
public:
    std::array<uint8_t, 16> ticketKeyEnc;
    std::array<uint8_t, 16> ticketKeyDec;

    static std::optional<std::unique_ptr<Ticket>> make_unique(const std::vector<uint8_t> &data, std::optional<const std::array<uint8_t, 16>> commonKey);

private:
    Ticket(const std::array<uint8_t, 16> &encryptedKey, const std::array<uint8_t, 16> &decryptedKey);
};