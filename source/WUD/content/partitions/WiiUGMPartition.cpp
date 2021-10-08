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

WiiUGMPartition::~WiiUGMPartition() {
    free(rawCert);
    free(rawTMD);
    free(rawTicket);
    delete basePartition;
}

WiiUGMPartition::WiiUGMPartition(WiiUPartition *partition, uint8_t *pRawTIK, uint32_t pTikLen, uint8_t *pRawTMD, uint32_t pTMDLen, uint8_t *pRawCert, uint32_t pCertLen) {
    basePartition = partition;
    rawCert = pRawCert;
    rawTMD = pRawTMD;
    rawTicket = pRawTIK;
    tikLen = pTikLen;
    TMDLen = pTMDLen;
    certLen = pCertLen;
}

std::string WiiUGMPartition::getVolumeId() const &{
    return basePartition->getVolumeId();
}

std::map<AddressInDiscBlocks, VolumeHeader *> WiiUGMPartition::getVolumes() const &{
    return basePartition->getVolumes();
}

uint16_t WiiUGMPartition::getFileSystemDescriptor() const {
    return basePartition->getFileSystemDescriptor();
}

uint64_t WiiUGMPartition::getSectionOffsetOnDefaultPartition() {
    return basePartition->getSectionOffsetOnDefaultPartition();
}
