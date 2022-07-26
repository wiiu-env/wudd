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
#include "NUSDataProviderWUD.h"

NUSDataProviderWUD::NUSDataProviderWUD(const std::shared_ptr<WiiUGMPartition> &pGamePartition, const std::shared_ptr<DiscReader> &pDiscReader) {
    gamePartition = pGamePartition;
    discReader    = pDiscReader;
}

NUSDataProviderWUD::~NUSDataProviderWUD() = default;

bool NUSDataProviderWUD::readRawContent(const std::shared_ptr<Content> &content, uint8_t *buffer, uint64_t offset, uint32_t size) {
    if (buffer == nullptr) {
        return false;
    }
    auto offsetInWUDOpt = getOffsetInWUD(content);
    if (!offsetInWUDOpt.has_value()) {
        return false;
    }
    auto offsetInWUD = offsetInWUDOpt.value() + offset;
    return discReader->readEncrypted(buffer, offsetInWUD, size);
}

bool NUSDataProviderWUD::getContentH3Hash(const std::shared_ptr<Content> &content, std::vector<uint8_t> &out_data) {
    auto cur = gamePartition->getVolumes().begin()->second->h3HashArrayList[content->index];
    if (cur == nullptr || cur->size == 0) {
        return false;
    }
    out_data.resize(cur->size);
    memcpy(out_data.data(), cur->data, cur->size);
    return true;
}

void NUSDataProviderWUD::setFST(const std::shared_ptr<FST> &pFST) {
    // We need to set the correct blocksizes
    auto blockSize = gamePartition->getVolumes().begin()->second->blockSize;
    for (const auto &e : pFST->sectionEntries->getSections()) {
        e->address = AddressInVolumeBlocks(blockSize, e->address.value);
        e->size    = SizeInVolumeBlocks(blockSize, e->size.value);
    }
    fst = pFST;
}

bool NUSDataProviderWUD::getRawCert(std::vector<uint8_t> &out_data) {
    out_data = gamePartition->rawCert;
    return true;
}

bool NUSDataProviderWUD::getRawTicket(std::vector<uint8_t> &out_data) {
    out_data = gamePartition->rawTicket;
    return true;
}

bool NUSDataProviderWUD::getRawTMD(std::vector<uint8_t> &out_data) {
    out_data = gamePartition->rawTMD;
    return true;
}

std::optional<uint64_t> NUSDataProviderWUD::getOffsetInWUD(const std::shared_ptr<Content> &content) const {
    if (content->index == 0) { // Index 0 is the FST which is at the beginning of the partition;
        auto vh = gamePartition->getVolumes().begin()->second;
        return gamePartition->getSectionOffsetOnDefaultPartition() + vh->FSTAddress.getAddressInBytes();
    }
    auto info = FSTUtils::getSectionEntryForIndex(fst, content->index);
    if (!info.has_value()) {
        DEBUG_FUNCTION_LINE("Failed to get section for Content");
        return {};
    }
    return gamePartition->getSectionOffsetOnDefaultPartition() + info.value()->address.getAddressInBytes();
}
