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
#include "DefaultNUSDataProcessor.h"

bool DefaultNUSDataProcessor::readPlainDecryptedContent(const std::shared_ptr<Content> &pContent, std::vector<uint8_t> &out_data) {
    if ((pContent->type & 0x0002) == 0x0002) {
        DEBUG_FUNCTION_LINE_ERR("Hashed content not supported yet");
        return false;
    }
    auto contentSize = ROUNDUP(pContent->encryptedFileSize, 16);
    out_data.resize(contentSize);

    if (contentSize > UINT32_MAX) {
        DEBUG_FUNCTION_LINE_ERR("Content is too big to read");
        OSFatal("Content is too big to read");
    }

    auto inData = make_unique_nothrow<uint8_t[]>((uint32_t) contentSize);
    if (!inData) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc");
        return false;
    }

    if (!dataProvider->readRawContent(pContent, inData.get(), 0, contentSize)) {
        DEBUG_FUNCTION_LINE_ERR("Failed tor read content");
        return false;
    }

    std::array<uint8_t, 16> IV{};
    memset(IV.data(), 0, 16);
    uint16_t content_index = pContent->index;
    memcpy(IV.data(), &content_index, 2);

    nusDecryption->decryptData(IV, inData.get(), out_data.data(), contentSize);
    return true;
}

std::shared_ptr<NUSDataProvider> &DefaultNUSDataProcessor::getDataProvider() {
    return dataProvider;
}

DefaultNUSDataProcessor::DefaultNUSDataProcessor(
        std::shared_ptr<NUSDataProvider> pDataProvider,
        std::shared_ptr<NUSDecryption> pNUSDecryption) : dataProvider(std::move(pDataProvider)),
                                                         nusDecryption(std::move(pNUSDecryption)) {
}
