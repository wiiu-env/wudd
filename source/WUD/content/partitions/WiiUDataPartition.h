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

#include "WiiUPartition.h"
#include <WUD/entities/FST/FST.h>
#include <cstdint>
#include <cstdlib>
#include <utils/blocksize/AddressInDiscBlocks.h>

class WiiUDataPartition : public WiiUPartition {

public:
    WiiUDataPartition(std::shared_ptr<WiiUPartition> pPartition, std::shared_ptr<FST> pFST);

    ~WiiUDataPartition() override;

    [[nodiscard]] const std::string &getVolumeId() const override;

    [[nodiscard]] const std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> &getVolumes() const override;

    [[nodiscard]] uint16_t getFileSystemDescriptor() const override;

    [[nodiscard]] uint64_t getSectionOffsetOnDefaultPartition() override;

private:
    std::shared_ptr<FST> fst;
    std::shared_ptr<WiiUPartition> basePartition;
};
