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
#include "Content.h"
#include <cstring>

Content::Content(uint32_t pId, uint16_t pIndex, uint16_t pType, uint64_t pEncryptedFileSize, const std::array<uint8_t, 0x14> &pHash) : ID(pId),
                                                                                                                                       index(pIndex),
                                                                                                                                       type(pType),
                                                                                                                                       encryptedFileSize(pEncryptedFileSize),
                                                                                                                                       hash(pHash) {
}

std::optional<std::shared_ptr<Content>> Content::make_shared(const std::array<uint8_t, 0x30> &data) {
    auto id                = ((uint32_t *) &data[0x00])[0];
    auto index             = ((uint16_t *) &data[0x04])[0];
    auto type              = ((uint16_t *) &data[0x06])[0];
    auto encryptedFileSize = ((uint64_t *) &data[0x08])[0];
    std::array<uint8_t, 0x14> hash{};
    memcpy(hash.data(), &data[0x10], 0x14);

    return std::shared_ptr<Content>(new Content(id, index, type, encryptedFileSize, hash));
}
