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

#include <cstdint>
#include <utils/blocksize/DiscBlockSize.h>
#include <utils/blocksize/AddressInDiscBlocks.h>
#include <WUD/entities/FST/FST.h>
#include <WUD/DiscReader.h>
#include "WiiUPartition.h"

#define WUD_TMD_FILENAME        "title.tmd"
#define WUD_TICKET_FILENAME     "title.tik"
#define WUD_CERT_FILENAME       "title.cert"

class WiiUPartitions {

public:
    static bool getFSTEntryAsByte(uint8_t **buffer_out, uint32_t *outSize, std::string &filePath, FST *fst, const AddressInDiscBlocks &volumeAddress, DiscReader *discReader);

    WiiUPartitions(DiscReader *reader, uint32_t offset, uint32_t numberOfPartitions, const DiscBlockSize &blockSize);

    ~WiiUPartitions();

    std::vector<WiiUPartition *> partitions;
    static uint32_t LENGTH;

};