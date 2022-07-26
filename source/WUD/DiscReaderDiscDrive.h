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

#include "DiscReader.h"
#include <cstdint>
#include <memory>
#include <optional>

class DiscReaderDiscDrive : public DiscReader {
public:
    DiscReaderDiscDrive();

    ~DiscReaderDiscDrive() override;

    static std::optional<std::unique_ptr<DiscReaderDiscDrive>> make_unique();

    bool readEncryptedSector(uint8_t *buffer, uint32_t block_cnt, uint32_t block_offset) const override;

    bool readEncrypted(uint8_t *buf, uint64_t offset, uint32_t size) override;

private:
    bool IsReady() override;
    bool init_done        = false;
    int32_t device_handle = -1;
};