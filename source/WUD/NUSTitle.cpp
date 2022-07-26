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
#include "NUSTitle.h"

#include <utility>

std::optional<std::shared_ptr<NUSTitle>> NUSTitle::loadTitle(const std::shared_ptr<NUSDataProvider> &dataProvider, const std::array<uint8_t, 16> &commonKey) {
    std::vector<uint8_t> dataBuffer;
    if (!dataProvider->getRawTMD(dataBuffer)) {
        DEBUG_FUNCTION_LINE("Failed to read TMD");
        return {};
    }

    auto tmdOpt = TitleMetaData::make_shared(dataBuffer);
    if (!tmdOpt.has_value()) {
        DEBUG_FUNCTION_LINE("Failed to parse TMD");
        return {};
    }
    dataBuffer.clear();

    if (!dataProvider->getRawTicket(dataBuffer)) {
        DEBUG_FUNCTION_LINE("Failed to read ticket data");
        return {};
    }

    auto ticketOpt = Ticket::make_shared(dataBuffer, commonKey);
    if (!ticketOpt.has_value()) {
        DEBUG_FUNCTION_LINE("Failed to parse ticket");
        return {};
    }

    dataBuffer.clear();
    auto decryption = std::make_shared<NUSDecryption>(std::move(ticketOpt.value()));
    auto dpp        = std::shared_ptr<NUSDataProcessor>(new DefaultNUSDataProcessor(dataProvider, decryption));

    // If we have more than one content, the index 0 is the FST.
    auto fstContentOpt = tmdOpt.value()->getContentByIndex(0);
    if (!fstContentOpt.has_value()) {
        DEBUG_FUNCTION_LINE("Failed to get content for index 0");
        return {};
    }

    if (!dpp->readPlainDecryptedContent(fstContentOpt.value(), dataBuffer)) {
        DEBUG_FUNCTION_LINE("Failed to read decrypted content");
        return {};
    }
    auto fstOpt = FST::make_shared(dataBuffer, 0, VolumeBlockSize(1));

    if (!fstOpt.has_value()) {
        DEBUG_FUNCTION_LINE("Failed to parse FST");
        return {};
    }

    // The dataprovider may need the FST to calculate the offset of a content
    // on the partition.
    dataProvider->setFST(fstOpt.value());
    return std::shared_ptr<NUSTitle>(new NUSTitle(tmdOpt.value(), dpp, dataProvider, decryption, ticketOpt.value(), fstOpt.value()));
}

NUSTitle::NUSTitle(std::shared_ptr<TitleMetaData> pTMD,
                   std::shared_ptr<NUSDataProcessor> pProcessor,
                   std::shared_ptr<NUSDataProvider> pDataProvider,
                   std::shared_ptr<NUSDecryption> pDecryption,
                   std::shared_ptr<Ticket> pTicket,
                   std::shared_ptr<FST> pFST) :

                                                dataProcessor(std::move(pProcessor)),
                                                tmd(std::move(pTMD)),
                                                ticket(std::move(pTicket)),
                                                fst(std::move(pFST)),
                                                decryption(std::move(pDecryption)),
                                                dataProvider(std::move(pDataProvider)) {
}

std::optional<std::shared_ptr<NUSTitle>>
NUSTitle::loadTitleFromGMPartition(const std::shared_ptr<WiiUGMPartition> &pPartition, const std::shared_ptr<DiscReader> &pDrive, const std::array<uint8_t, 16> &commonKey) {
    return loadTitle(std::shared_ptr<NUSDataProvider>(new NUSDataProviderWUD(pPartition, pDrive)), commonKey);
}

NUSTitle::~NUSTitle() = default;
