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
#include <cstring>
#include "Content.h"

uint32_t Content::LENGTH = 0x30;

Content::Content(uint8_t *data) {
    ID = ((uint32_t *) &data[0x00])[0];
    index = ((uint16_t *) &data[0x04])[0];
    type = ((uint16_t *) &data[0x06])[0];
    encryptedFileSize = ((uint64_t *) &data[0x08])[0];
    memcpy(hash, &data[0x10], 0x14);
}
