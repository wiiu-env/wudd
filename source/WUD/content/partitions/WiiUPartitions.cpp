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
#include <utils/FSTUtils.h>
#include <coreinit/debug.h>
#include "WiiUPartitions.h"
#include "WiiUGMPartition.h"
#include "WiiUDataPartition.h"

uint32_t WiiUPartitions::LENGTH = 30720;

WiiUPartitions::WiiUPartitions(DiscReader *reader, uint32_t offset, uint32_t numberOfPartitions, const DiscBlockSize &blockSize) {
    std::vector<WiiUPartition *> tmp;
    tmp.reserve(numberOfPartitions);
    for (uint32_t i = 0; i < numberOfPartitions; i++) {
        tmp.push_back(new WiiUPartition(reader, offset + (i * 128), blockSize));
    }
    WiiUPartition *SIPartition = nullptr;
    for (auto &partition: tmp) {
        if (partition->getVolumeId().starts_with("SI")) {
            SIPartition = partition;
        }
    }

    if (SIPartition != nullptr) {
        for (auto const&[key, val]: SIPartition->getVolumes()) {
            auto volumeAddress = key;
            auto volumeAddressInBytes = volumeAddress.getAddressInBytes();
            auto volumeHeader = val;

            auto fst = (uint8_t *) malloc(volumeHeader->FSTSize);
            if (fst == nullptr) {
                OSFatal("WiiUPartitions: Failed to alloc FST buffer");
            }

            if (!reader->hasDiscKey) {
                if (!reader->readEncrypted(fst, volumeAddressInBytes + volumeHeader->FSTAddress.getAddressInBytes(),
                                           volumeHeader->FSTSize)) {
                    OSFatal("WiiUPartitions: Failed to read encrypted");
                }
            } else {
                if (!reader->readDecrypted(fst, volumeAddressInBytes + volumeHeader->FSTAddress.getAddressInBytes(), 0, volumeHeader->FSTSize,
                                           reader->discKey, nullptr, true)) {
                    OSFatal("WiiUPartitions: Failed to read decrypted");
                }
            }

            FST *siFST = new FST(fst, volumeHeader->FSTSize, 0, volumeHeader->blockSize);
            free(fst);

            for (auto &child: siFST->getRootEntry()->getDirChildren()) {
                uint8_t *tikRaw = nullptr;
                uint32_t tikRawLen = 0;
                std::string tikFilePath = std::string(child->getFullPath() + '/' + WUD_TICKET_FILENAME);
                if (!getFSTEntryAsByte(&tikRaw, &tikRawLen, tikFilePath, siFST, volumeAddress, reader)) {
                    OSFatal("tikRaw");
                }

                uint8_t *tmdRaw = nullptr;
                uint32_t tmdRawLen = 0;
                std::string tmdFilePath = std::string(child->getFullPath() + '/' + WUD_TMD_FILENAME);
                if (!getFSTEntryAsByte(&tmdRaw, &tmdRawLen, tmdFilePath, siFST, volumeAddress, reader)) {
                    OSFatal("tmdRaw");
                }

                uint8_t *certRaw = nullptr;
                uint32_t certRawLen = 0;
                std::string certFilePath = std::string(child->getFullPath() + '/' + WUD_CERT_FILENAME);
                if (!getFSTEntryAsByte(&certRaw, &certRawLen, certFilePath, siFST, volumeAddress, reader)) {
                    OSFatal("certRaw");
                }

                char partitionNameRaw[0x12];
                memset(partitionNameRaw, 0, 0x12);
                snprintf(partitionNameRaw, 0x11, "%016llX", *((uint64_t *) &tikRaw[0x1DC]));

                std::string partitionName = std::string("GM") + partitionNameRaw;

                WiiUPartition *curPartition = nullptr;
                for (auto &partition: tmp) {
                    if (partition->getVolumeId().starts_with(partitionName)) {
                        curPartition = partition;
                    }
                }

                if (curPartition == nullptr) {
                    OSFatal("Failed to get partition");
                }

                auto *gmPartition = new WiiUGMPartition(curPartition, tikRaw, tikRawLen, tmdRaw, tmdRawLen, certRaw, certRawLen);
                partitions.push_back(gmPartition);
            }
            delete siFST;
        }

    }

    for (auto &partition: tmp) {
        if (partition->getVolumeId().starts_with("GM")) {
            continue;
        }
        if (partition->getVolumes().size() != 1) {
            OSFatal("We can't handle more or less than one partion address yet.");
        }
        auto volumeAddress = partition->getVolumes().begin()->first;
        auto vh = partition->getVolumes().begin()->second;
        auto *rawFST = (uint8_t *) malloc(vh->FSTSize);
        if (rawFST == nullptr) {
            OSFatal("Failed to alloc rawFST");
        }
        if (!reader->hasDiscKey) {
            if (!reader->readEncrypted(rawFST, volumeAddress.getAddressInBytes() + vh->FSTAddress.getAddressInBytes(), vh->FSTSize)) {
                OSFatal("WiiUPartition: Failed to read encrypted");
            }
        } else {
            if (!reader->readDecrypted(rawFST, volumeAddress.getAddressInBytes() + vh->FSTAddress.getAddressInBytes(), 0, vh->FSTSize,
                                       reader->discKey, nullptr, true)) {
                OSFatal("WiiUPartition: Failed to read encrypted");
            }
        }

        FST *fst = new FST(rawFST, vh->FSTSize, 0, vh->blockSize);
        free(rawFST);
        partitions.push_back(new WiiUDataPartition(partition, fst));
    }
}

WiiUPartitions::~WiiUPartitions() {
    for (auto &partition: partitions) {
        delete partition;
    }
}

bool WiiUPartitions::getFSTEntryAsByte(uint8_t **buffer_out, uint32_t *outSize, std::string &filePath, FST *fst, const AddressInDiscBlocks &volumeAddress, DiscReader *discReader) {
    NodeEntry *entry = FSTUtils::getFSTEntryByFullPath(fst->nodeEntries->rootEntry, filePath);

    auto asFileEntry = dynamic_cast<FileEntry *>(entry);
    if (asFileEntry == nullptr) {
        return false;
    }

    SectionEntry *info = asFileEntry->getSectionEntry();
    if (info == nullptr) {
        OSFatal("WiiUPartitions::getFSTEntryAsByte, section info was null");
    }

    uint64_t sectionOffsetOnDisc = volumeAddress.getAddressInBytes() + info->address.getAddressInBytes();

    auto *buffer = (uint8_t *) malloc(asFileEntry->getSize());
    if (buffer == nullptr) {
        return false;
    }
    *buffer_out = buffer;
    *outSize = asFileEntry->getSize();

    if (!discReader->hasDiscKey) {
        return discReader->readEncrypted(buffer, sectionOffsetOnDisc + asFileEntry->getOffset(), asFileEntry->getSize());
    }

    // Calculating the IV
    uint8_t IV[16];
    memset(IV, 0, 16);
    uint64_t ivTemp = asFileEntry->getOffset() >> 16;
    memcpy(IV + 8, &ivTemp, 8);

    return discReader->readDecrypted(buffer, sectionOffsetOnDisc, asFileEntry->getOffset(), asFileEntry->getSize(), discReader->discKey, IV, false);
}

