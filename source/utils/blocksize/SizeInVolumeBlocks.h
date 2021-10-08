#pragma once

#include "VolumeBlockSize.h"
#include "SizeInBlocks.h"

class SizeInVolumeBlocks : public SizeInBlocks {

public:
    SizeInVolumeBlocks() = default;

    SizeInVolumeBlocks(const VolumeBlockSize &blockSize, uint32_t value) : SizeInBlocks(blockSize, value) {

    }
};