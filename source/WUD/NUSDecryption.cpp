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
#include "NUSDecryption.h"

#include <utility>
#include <utils/logger.h>

void NUSDecryption::decryptData(const std::array<uint8_t, 0x10> &IV, uint8_t *inData, uint8_t *outData, uint32_t size) const {
    aes_set_key(ticket->ticketKeyDec.data());
    aes_decrypt((uint8_t *) IV.data(), inData, outData, size);
}

NUSDecryption::NUSDecryption(std::unique_ptr<Ticket> pTicket) : ticket(std::move(pTicket)) {
}