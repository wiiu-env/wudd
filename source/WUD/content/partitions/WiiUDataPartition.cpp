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
#include "WiiUDataPartition.h"

#include <utility>

WiiUDataPartition::~WiiUDataPartition() = default;

WiiUDataPartition::WiiUDataPartition(
        std::shared_ptr<WiiUPartition> pPartition,
        std::shared_ptr<FST> pFST) : fst(std::move(pFST)),
                                     basePartition(std::move(pPartition)) {
}

const std::string &WiiUDataPartition::getVolumeId() const {
    return basePartition->getVolumeId();
}

const std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> &WiiUDataPartition::getVolumes() const {
    return basePartition->getVolumes();
}

uint16_t WiiUDataPartition::getFileSystemDescriptor() const {
    return basePartition->getFileSystemDescriptor();
}

uint64_t WiiUDataPartition::getSectionOffsetOnDefaultPartition() {
    return basePartition->getSectionOffsetOnDefaultPartition();
}
