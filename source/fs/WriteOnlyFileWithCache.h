/****************************************************************************
 * Copyright (C) 2021 Maschell
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

#include <fs/CFile.hpp>

#define SEEK_SET_BASE_CLASS 0x4242

class WriteOnlyFileWithCache : public CFile {
public:
    WriteOnlyFileWithCache(const char *string, int32_t cacheSize, bool split = false);

    ~WriteOnlyFileWithCache() override;

    int32_t write(const uint8_t *data, size_t size) override;

    int32_t seek(int64_t offset, int32_t origin) override;

    int32_t read(uint8_t *ptr, size_t size) override;

    [[nodiscard]] uint64_t tell() const override {
        return pos + writeBufferPos;
    };

    bool flush();

    void *writeBuffer = nullptr;
    size_t writeBufferSize;
    size_t writeBufferPos;

    bool splitFile = false;
    int32_t part   = 1;
    std::string originalPath;
};
