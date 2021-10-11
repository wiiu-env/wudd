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

#include <memory>
#include <cstdint>
#include <cstdlib>
#include "WiiUPartition.h"

class WiiUGMPartition : public WiiUPartition {
public:
    WiiUGMPartition(std::shared_ptr<WiiUPartition> partition,
                    std::vector<uint8_t> pRawTicket,
                    std::vector<uint8_t> pRawTMD,
                    std::vector<uint8_t> pRawCert);

    [[nodiscard]] std::string getVolumeId() const & override;

    [[nodiscard]] std::map<AddressInDiscBlocks, std::shared_ptr<VolumeHeader>> getVolumes() const & override;

    [[nodiscard]] uint16_t getFileSystemDescriptor() const override;

    [[nodiscard]] uint64_t getSectionOffsetOnDefaultPartition() override;

    std::vector<uint8_t> rawTicket;
    std::vector<uint8_t> rawTMD;
    std::vector<uint8_t> rawCert;

private:
    std::shared_ptr<WiiUPartition> basePartition;
};

