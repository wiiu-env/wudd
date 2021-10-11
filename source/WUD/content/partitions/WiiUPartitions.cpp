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

#include <utility>
#include "WiiUPartitions.h"
#include "WiiUGMPartition.h"
#include "WiiUDataPartition.h"

bool WiiUPartitions::getFSTEntryAsByte(std::string &filePath,
                                       const std::shared_ptr<FST> &fst,
                                       const AddressInDiscBlocks &volumeAddress,
                                       const std::shared_ptr<DiscReader> &discReader,
                                       std::vector<uint8_t> &out_data) {
    auto entryOpt = FSTUtils::getFSTEntryByFullPath(fst->getRootEntry(), filePath);
    if (!entryOpt.has_value()) {
        return false;
    }

    auto asFileEntry = std::dynamic_pointer_cast<FileEntry>(entryOpt.value());
    if (asFileEntry == nullptr) {
        return false;
    }

    auto info = asFileEntry->getSectionEntry();
    uint64_t sectionOffsetOnDisc = volumeAddress.getAddressInBytes() + info->address.getAddressInBytes();

    out_data.resize(asFileEntry->getSize());

    if (!discReader->hasDiscKey) {
        return discReader->readEncrypted(out_data.data(), sectionOffsetOnDisc + asFileEntry->getOffset(), asFileEntry->getSize());
    }

    // Calculating the IV
    uint8_t IV[16];
    memset(IV, 0, 16);
    uint64_t ivTemp = asFileEntry->getOffset() >> 16;
    memcpy(IV + 8, &ivTemp, 8);

    return discReader->readDecrypted(out_data.data(), sectionOffsetOnDisc, asFileEntry->getOffset(), asFileEntry->getSize(), discReader->discKey, IV, false);
}

std::optional<std::unique_ptr<WiiUPartitions>>
WiiUPartitions::make_unique(const std::shared_ptr<DiscReader> &discReader, uint32_t offset, uint32_t numberOfPartitions, const DiscBlockSize &blockSize) {
    std::vector<std::shared_ptr<WiiUPartition>> tmp;
    std::vector<std::shared_ptr<WiiUPartition>> partitions;
    partitions.reserve(numberOfPartitions);
    tmp.reserve(numberOfPartitions);
    for (uint32_t i = 0; i < numberOfPartitions; i++) {
        auto partitionOpt = WiiUPartition::make_shared(discReader, offset + (i * 128), blockSize);
        if (!partitionOpt.has_value()) {
            DEBUG_FUNCTION_LINE("Failed to read partition");
            return {};
        }
        tmp.push_back(partitionOpt.value());
    }
    std::optional<std::shared_ptr<WiiUPartition>> SIPartition;
    for (auto &partition: tmp) {
        if (partition->getVolumeId().starts_with("SI")) {
            SIPartition = partition;
            break;
        }
    }

    if (SIPartition.has_value()) {
        for (auto const&[key, val]: SIPartition.value()->getVolumes()) {
            auto volumeAddress = key;
            auto volumeAddressInBytes = volumeAddress.getAddressInBytes();
            auto volumeHeader = val;

            std::vector<uint8_t> fstData;
            fstData.resize(volumeHeader->FSTSize);

            if (!discReader->hasDiscKey) {
                if (!discReader->readEncrypted(fstData.data(), volumeAddressInBytes + volumeHeader->FSTAddress.getAddressInBytes(),
                                               volumeHeader->FSTSize)) {
                    DEBUG_FUNCTION_LINE("Failed to read FST");
                    return {};
                }
            } else {
                if (!discReader->readDecrypted(fstData.data(), volumeAddressInBytes + volumeHeader->FSTAddress.getAddressInBytes(), 0, volumeHeader->FSTSize,
                                               discReader->discKey, nullptr, true)) {
                    DEBUG_FUNCTION_LINE("Failed to read FST");
                    return {};
                }
            }

            auto siFST = FST::make_shared(fstData, 0, volumeHeader->blockSize);
            if (!siFST.has_value()) {
                DEBUG_FUNCTION_LINE("Failed to parse FST");
                return {};
            }

            for (auto &child: siFST.value()->getRootEntry()->getDirChildren()) {
                std::vector<uint8_t> bufferTicket;
                std::string tikFilePath = std::string(child->getFullPath() + '/' + WUD_TICKET_FILENAME);
                if (!getFSTEntryAsByte(tikFilePath, siFST.value(), volumeAddress, discReader, bufferTicket)) {
                    DEBUG_FUNCTION_LINE("Failted to read FSTEntry");
                    return {};
                }

                std::vector<uint8_t> bufferTMD;
                std::string tmdFilePath = std::string(child->getFullPath() + '/' + WUD_TMD_FILENAME);
                if (!getFSTEntryAsByte(tmdFilePath, siFST.value(), volumeAddress, discReader, bufferTMD)) {
                    DEBUG_FUNCTION_LINE("Failted to read FSTEntry");
                    return {};
                }

                std::vector<uint8_t> bufferCert;
                std::string certFilePath = std::string(child->getFullPath() + '/' + WUD_CERT_FILENAME);
                if (!getFSTEntryAsByte(certFilePath, siFST.value(), volumeAddress, discReader, bufferCert)) {
                    DEBUG_FUNCTION_LINE("Failted to read FSTEntry");
                    return {};
                }

                char partitionNameRaw[0x12];
                memset(partitionNameRaw, 0, 0x12);
                snprintf(partitionNameRaw, 0x11, "%016llX", *((uint64_t *) &bufferTicket[0x1DC]));

                std::string partitionName = std::string("GM") + partitionNameRaw;

                std::optional<std::shared_ptr<WiiUPartition>> curPartition;
                for (auto &partition: tmp) {
                    if (partition->getVolumeId().starts_with(partitionName)) {
                        curPartition = partition;
                        break;
                    }
                }

                if (!curPartition.has_value()) {
                    DEBUG_FUNCTION_LINE("Failed to find partition");
                    return {};
                }

                auto gmPartition = std::shared_ptr<WiiUPartition>(new WiiUGMPartition(curPartition.value(), bufferTicket, bufferTMD, bufferCert));
                partitions.push_back(gmPartition);
            }
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

        std::vector<uint8_t> fstData;
        fstData.resize(vh->FSTSize);

        if (!discReader->hasDiscKey) {
            if (!discReader->readEncrypted(fstData.data(), volumeAddress.getAddressInBytes() + vh->FSTAddress.getAddressInBytes(), vh->FSTSize)) {
                OSFatal("WiiUPartition: Failed to read encrypted");
            }
        } else {
            if (!discReader->readDecrypted(fstData.data(), volumeAddress.getAddressInBytes() + vh->FSTAddress.getAddressInBytes(), 0, vh->FSTSize,
                                           discReader->discKey, nullptr, true)) {
                OSFatal("WiiUPartition: Failed to read encrypted");
            }
        }

        auto fstOpt = FST::make_shared(fstData, 0, vh->blockSize);
        if (!fstOpt.has_value()) {
            DEBUG_FUNCTION_LINE("Failed to parse FST");
            return {};
        }
        partitions.push_back(std::shared_ptr<WiiUPartition>(new WiiUDataPartition(partition, fstOpt.value())));
    }
    return std::unique_ptr<WiiUPartitions>(new WiiUPartitions(partitions));
}

WiiUPartitions::WiiUPartitions(std::vector<std::shared_ptr<WiiUPartition>> pPartitions) : partitions(std::move(pPartitions)) {
    DEBUG_FUNCTION_LINE();
}

