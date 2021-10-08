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
#include <cstdlib>
#include "WiiUPartition.h"

class WiiUGMPartition : public WiiUPartition {
public:
    WiiUGMPartition(WiiUPartition *partition, uint8_t *pRawTIK, uint32_t pTikLen, uint8_t *pRawTMD, uint32_t pTMDLen, uint8_t *pRawCert, uint32_t pCertLen);

    ~WiiUGMPartition() override;

    [[nodiscard]] std::string getVolumeId() const & override;

    [[nodiscard]] std::map<AddressInDiscBlocks, VolumeHeader *> getVolumes() const & override;

    [[nodiscard]] uint16_t getFileSystemDescriptor() const override;

    [[nodiscard]] uint64_t getSectionOffsetOnDefaultPartition() override;

    uint8_t *rawTicket;
    uint8_t *rawTMD;
    uint8_t *rawCert;
    uint32_t tikLen;
    uint32_t TMDLen;
    uint32_t certLen;

private:
    WiiUPartition *basePartition;
};

