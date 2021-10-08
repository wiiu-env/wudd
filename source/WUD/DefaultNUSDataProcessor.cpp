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

bool DefaultNUSDataProcessor::readPlainDecryptedContent(Content *pContent, uint8_t **data, uint32_t *length) {
    if (pContent == nullptr || data == nullptr || length == nullptr) {
        return false;
    }

    if ((pContent->type & 0x0002) == 0x0002) {
        DEBUG_FUNCTION_LINE("Hashed content not supported yet");
        return false;
    }

    *length = ROUNDUP(pContent->encryptedFileSize, 16);
    *data = (uint8_t *) malloc(*length);
    if (*data == nullptr) {
        return false;
    }

    auto *inData = (uint8_t *) malloc(*length);

    if (!dataProvider->readRawContent(pContent, inData, 0, *length)) {
        free(*data);
        free(inData);
        return false;
    }

    uint8_t IV[16];
    memset(IV, 0, 16);
    uint16_t content_index = pContent->index;
    memcpy(IV, &content_index, 2);

    nusDecryption->decryptData(IV, inData, *data, *length);
    free(inData);
    return true;
}

NUSDataProvider *DefaultNUSDataProcessor::getDataProvider() {
    return dataProvider;
}

DefaultNUSDataProcessor::DefaultNUSDataProcessor(NUSDataProvider *pDataProvider, NUSDecryption *pNUSDecryption) {
    dataProvider = pDataProvider;
    nusDecryption = pNUSDecryption;
}
