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
#include "NUSDataProvider.h"
#include <WUD/content/partitions/WiiUGMPartition.h>
#include <coreinit/debug.h>
#include <utils/FSTUtils.h>

class NUSDataProviderWUD : public NUSDataProvider {

public:
    NUSDataProviderWUD(std::shared_ptr<WiiUGMPartition> pPartition, std::shared_ptr<DiscReader> pDiscReader);

    ~NUSDataProviderWUD() override;

    bool readRawContent(const std::shared_ptr<Content> &content, uint8_t *buffer, uint64_t offset, uint32_t size) override;

    bool getContentH3Hash(const std::shared_ptr<Content> &content, std::vector<uint8_t> &out_data) override;

    void setFST(const std::shared_ptr<FST> &pFST) override;

    bool getRawCert(std::vector<uint8_t> &out_data) override;

    bool getRawTicket(std::vector<uint8_t> &data) override;

    bool getRawTMD(std::vector<uint8_t> &out_data) override;

    std::shared_ptr<FST> fst;
    std::shared_ptr<WiiUGMPartition> gamePartition;
    std::shared_ptr<DiscReader> discReader;
};