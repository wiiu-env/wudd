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

Ticket::Ticket(uint8_t *data, uint8_t *commonKey) {
    uint8_t *tikKeyEnc = data + 0x1BF;
    uint8_t *title_id = data + 0x1DC;
    uint8_t IV[0x10];

    int k;
    for (k = 0; k < 8; k++) {
        IV[k] = title_id[k];
        IV[k + 8] = 0x00;
    }

    aes_set_key(commonKey);
    aes_decrypt(IV, tikKeyEnc, ticketKeyDec, 16);
    memcpy(ticketKeyEnc, tikKeyEnc, 16);
}
