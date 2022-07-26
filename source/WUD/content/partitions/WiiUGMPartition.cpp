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
#include "WiiUGMPartition.h"
#include "WUD/entities/FST/FST.h"
#include "WUD/entities/TMD/Content.h"

WiiUGMPartition::WiiUGMPartition(std::unique_ptr<WiiUPartition> partition,
                                 std::vector<uint8_t> pRawTicket,
                                 std::vector<uint8_t> pRawTMD,
                                 std::vector<uint8_t> pRawCert,
                                 std::string pathOnSIPartition)
    : WiiUPartition(),
      rawTicket(std::move(pRawTicket)),
      rawTMD(std::move(pRawTMD)),
      rawCert(std::move(pRawCert)),
      pathOnSIPartition(std::move(pathOnSIPartition)),
      basePartition(std::move(partition)) {
}

const std::string &WiiUGMPartition::getVolumeId() const {
    return basePartition->getVolumeId();
}

const std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> &WiiUGMPartition::getVolumes() const {
    return basePartition->getVolumes();
}

uint16_t WiiUGMPartition::getFileSystemDescriptor() const {
    return basePartition->getFileSystemDescriptor();
}

uint64_t WiiUGMPartition::getSectionOffsetOnDefaultPartition() {
    return basePartition->getSectionOffsetOnDefaultPartition();
}