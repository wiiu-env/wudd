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
#include "Ticket.h"
#include <algorithm>
#include <utils/logger.h>

Ticket::Ticket(const std::array<uint8_t, 16> &pEncryptedKey, const std::array<uint8_t, 16> &pDecryptedKey) : ticketKeyEnc(pEncryptedKey),
                                                                                                             ticketKeyDec(pDecryptedKey) {
}

std::optional<std::unique_ptr<Ticket>> Ticket::make_unique(const std::vector<uint8_t> &data, std::optional<const std::array<uint8_t, 16>> commonKey) {
    if (data.size() <= 0x1DC + 0x10) {
        DEBUG_FUNCTION_LINE_ERR("Not enough data to parse a ticket");
        return {};
    }

    std::array<uint8_t, 16> title_id{};
    std::array<uint8_t, 16> decryptedKey{};
    std::array<uint8_t, 16> encryptedKey{};

    std::copy_n(data.begin() + 0x1BF, 0x10, decryptedKey.begin());
    std::copy_n(data.begin() + 0x1BF, 0x10, encryptedKey.begin());
    std::copy_n(data.begin() + 0x1DC, 0x10, title_id.begin());

    uint8_t IV[0x10];
    for (int i = 0; i < 8; i++) {
        IV[i]     = title_id[i];
        IV[i + 8] = 0x00;
    }

    if (commonKey.has_value()) {
        aes_set_key((uint8_t *) commonKey.value().data());
        aes_decrypt(IV, encryptedKey.data(), decryptedKey.data(), 16);
    }
    auto ticket = new (std::nothrow) Ticket(encryptedKey, decryptedKey);
    if (!ticket) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate Ticket");
        return {};
    }
    return std::unique_ptr<Ticket>(ticket);
}
