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

#include <memory>
#include <utility>

std::optional<std::unique_ptr<NUSTitle>> NUSTitle::make_unique(std::unique_ptr<NUSDataProvider> dataProvider, const std::array<uint8_t, 16> &commonKey) {
    std::vector<uint8_t> dataBuffer;
    if (!dataProvider->getRawTMD(dataBuffer)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read TMD");
        return {};
    }

    auto tmdOpt = TitleMetaData::make_unique(dataBuffer);
    if (!tmdOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse TMD");
        return {};
    }
    dataBuffer.clear();

    if (!dataProvider->getRawTicket(dataBuffer)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read ticket data");
        return {};
    }

    auto ticketOpt = Ticket::make_unique(dataBuffer, commonKey);
    if (!ticketOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse ticket");
        return {};
    }

    dataBuffer.clear();

    std::shared_ptr<NUSDataProvider> dataProviderShared = std::move(dataProvider);

    auto decryption = std::make_shared<NUSDecryption>(std::move(ticketOpt.value()));
    auto dpp        = std::unique_ptr<NUSDataProcessor>(new DefaultNUSDataProcessor(dataProviderShared, decryption));

    // If we have more than one content, the index 0 is the FST.
    auto fstContentOpt = tmdOpt.value()->getContentByIndex(0);
    if (!fstContentOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get content for index 0");
        return {};
    }

    if (!dpp->readPlainDecryptedContent(fstContentOpt.value(), dataBuffer)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read decrypted content");
        return {};
    }
    auto fstOpt = FST::make_shared(dataBuffer, 0, VolumeBlockSize(1));

    if (!fstOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse FST");
        return {};
    }

    // The dataprovider may need the FST to calculate the offset of a content
    // on the partition.
    dataProviderShared->setFST(fstOpt.value());

    return std::unique_ptr<NUSTitle>(new NUSTitle(
            std::move(tmdOpt.value()),
            std::move(dpp),
            dataProviderShared,
            decryption,
            std::move(ticketOpt.value()),
            fstOpt.value()));
}

NUSTitle::NUSTitle(std::unique_ptr<TitleMetaData> pTMD,
                   std::unique_ptr<NUSDataProcessor> pProcessor,
                   std::shared_ptr<NUSDataProvider> pDataProvider,
                   std::shared_ptr<NUSDecryption> pDecryption,
                   std::unique_ptr<Ticket> pTicket,
                   std::shared_ptr<FST> pFST) :

                                                dataProcessor(std::move(pProcessor)),
                                                tmd(std::move(pTMD)),
                                                ticket(std::move(pTicket)),
                                                fst(std::move(pFST)),
                                                decryption(std::move(pDecryption)),
                                                dataProvider(std::move(pDataProvider)) {
}

std::optional<std::unique_ptr<NUSTitle>>
NUSTitle::loadTitleFromGMPartition(std::shared_ptr<WiiUGMPartition> pPartition, std::shared_ptr<DiscReader> pDiscReader, const std::array<uint8_t, 16> &commonKey) {
    return make_unique(std::unique_ptr<NUSDataProvider>(new NUSDataProviderWUD(std::move(pPartition), std::move(pDiscReader))), commonKey);
}

std::string NUSTitle::getLongnameEn() {
    if (!longname_en) {
        auto *xml = (ACPMetaXml *) memalign(0x40, sizeof(ACPMetaXml));
        if (xml) {
            if (ACPGetTitleMetaXml(tmd->titleId, xml) == ACP_RESULT_SUCCESS) {
                std::string tmp = xml->longname_en;
                StringTools::StripUnicodeAndLineBreak(tmp);
                longname_en = tmp;
            }
            free(xml);
        } else {
            longname_en = "Unknown";
        }

        if (!longname_en || longname_en->empty() || longname_en.value() == " ") {
            longname_en = "Unknown";
        }
    }
    return longname_en.value();
}

std::string NUSTitle::getShortnameEn() {
    if (!shortname_en) {
        auto *xml = (ACPMetaXml *) memalign(0x40, sizeof(ACPMetaXml));
        if (xml) {
            if (ACPGetTitleMetaXml(tmd->titleId, xml) == ACP_RESULT_SUCCESS) {
                std::string tmp = xml->shortname_en;
                StringTools::StripUnicodeAndLineBreak(tmp);
                shortname_en = tmp;
            }
            free(xml);
        } else {
            shortname_en = "Unknown";
        }
        if (!shortname_en || shortname_en->empty() || shortname_en.value() == " ") {
            shortname_en = "Unknown";
        }
    }
    return shortname_en.value();
}

NUSTitle::~NUSTitle() = default;
