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

NUSDataProviderWUD::NUSDataProviderWUD(WiiUGMPartition *pGamePartition, DiscReader *pDiscReader) {
    gamePartition = pGamePartition;
    discReader = pDiscReader;
}

NUSDataProviderWUD::~NUSDataProviderWUD() {
    delete fst;
}

bool NUSDataProviderWUD::readRawContent(Content *content, uint8_t *buffer, uint64_t offset, uint32_t size) {
    if (buffer == nullptr) {
        DEBUG_FUNCTION_LINE();
        return false;
    }
    auto offsetInWUD = getOffsetInWUD(content) + offset;
    return discReader->readEncrypted(buffer, offsetInWUD, size);
}

bool NUSDataProviderWUD::getContentH3Hash(Content *content, uint8_t **data, uint32_t *size) {
    if (content == nullptr || data == nullptr || size == nullptr) {
        DEBUG_FUNCTION_LINE();
        return false;
    }
    auto cur = gamePartition->getVolumes().begin()->second->h3HashArrayList[content->index];
    if (cur == nullptr || cur->size == 0) {
        DEBUG_FUNCTION_LINE();
        return false;
    }
    *data = (uint8_t *) malloc(cur->size);
    *size = cur->size;
    memcpy(*data, cur->data, *size);
    return true;
}

void NUSDataProviderWUD::setFST(FST *pFST) {
    // We need to set the correct blocksizes
    auto blockSize = gamePartition->getVolumes().begin()->second->blockSize;
    for (const auto &e: pFST->sectionEntries->getSections()) {
        e->address = AddressInVolumeBlocks(blockSize, e->address.value);
        e->size = SizeInVolumeBlocks(blockSize, e->size.value);
    }
    fst = pFST;
}

bool NUSDataProviderWUD::getRawCert(uint8_t **data, uint32_t *size) {
    if (data == nullptr || size == nullptr) {
        return false;
    }
    *data = (uint8_t *) malloc(gamePartition->certLen);
    if (*data == nullptr) {
        return false;
    }
    *size = gamePartition->certLen;
    memcpy(*data, gamePartition->rawCert, gamePartition->certLen);
    return true;
}

bool NUSDataProviderWUD::getRawTicket(uint8_t **data, uint32_t *size) {
    if (data == nullptr || size == nullptr) {
        return false;
    }
    *data = (uint8_t *) malloc(gamePartition->tikLen);
    if (*data == nullptr) {
        return false;
    }
    *size = gamePartition->tikLen;
    memcpy(*data, gamePartition->rawTicket, gamePartition->tikLen);
    return true;
}

bool NUSDataProviderWUD::getRawTMD(uint8_t **data, uint32_t *size) {
    if (data == nullptr || size == nullptr) {
        DEBUG_FUNCTION_LINE("input was null");
        return false;
    }
    *data = (uint8_t *) malloc(gamePartition->TMDLen);
    if (*data == nullptr) {
        DEBUG_FUNCTION_LINE("Failed to alloc memory");
        return false;
    }
    *size = gamePartition->TMDLen;
    memcpy(*data, gamePartition->rawTMD, gamePartition->TMDLen);
    return true;
}

uint64_t NUSDataProviderWUD::getOffsetInWUD(Content *content) const {
    if (content->index == 0) { // Index 0 is the FST which is at the beginning of the partition;
        auto *vh = gamePartition->getVolumes().begin()->second;
        return gamePartition->getSectionOffsetOnDefaultPartition() + vh->FSTAddress.getAddressInBytes();
    }
    auto *info = FSTUtils::getSectionEntryForIndex(fst, content->index);
    if (info == nullptr) {
        OSFatal("Failed to get section for Content");
    }
    return gamePartition->getSectionOffsetOnDefaultPartition() + info->address.getAddressInBytes();
}
