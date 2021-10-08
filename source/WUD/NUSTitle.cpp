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

NUSTitle *NUSTitle::loadTitle(NUSDataProvider *dataProvider, uint8_t *commonKey) {
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    if (!dataProvider->getRawTMD(&data, &dataLen)) {
        DEBUG_FUNCTION_LINE("Failed to read TMD");
        delete dataProvider;
        return nullptr;
    }

    auto *tmd = new TitleMetaData(data);
    free(data);

    if (!dataProvider->getRawTicket(&data, &dataLen)) {
        DEBUG_FUNCTION_LINE("Failed to read ticket");
        delete tmd;
        delete dataProvider;
        return nullptr;
    }

    auto *ticket = new Ticket(data, commonKey);
    free(data);
    auto *decryption = new NUSDecryption(ticket);
    auto *dpp = new DefaultNUSDataProcessor(dataProvider, decryption);

    // If we have more than one content, the index 0 is the FST.
    Content *fstContent = tmd->getContentByIndex(0);

    if (!dpp->readPlainDecryptedContent(fstContent, &data, &dataLen)) {
        DEBUG_FUNCTION_LINE("Failed to read decrypted content");
        delete dataProvider;
        delete dpp;
        delete decryption;
        delete ticket;
        delete tmd;
        return nullptr;
    }
    FST *fst = new FST(data, dataLen, 0, VolumeBlockSize(1));

    // The dataprovider may need the FST to calculate the offset of a content
    // on the partition.
    dataProvider->setFST(fst);

    return new NUSTitle(tmd, dpp, dataProvider, decryption, ticket, fst);
}

NUSTitle::NUSTitle(TitleMetaData *pTMD, NUSDataProcessor *pProcessor, NUSDataProvider *pDataProvider, NUSDecryption *pDecryption, Ticket *pTicket, FST *pFST) {
    tmd = pTMD;
    dataProcessor = pProcessor;
    ticket = pTicket;
    fst = pFST;
    decryption = pDecryption;
    dataProvider = pDataProvider;
}

NUSTitle::~NUSTitle() {
    delete dataProvider;
    delete dataProcessor;
    delete decryption;
    delete ticket;
    delete tmd;
}

NUSTitle *NUSTitle::loadTitleFromGMPartition(WiiUGMPartition *pPartition, DiscReaderDiscDrive *pDrive, uint8_t *commonKey) {
    return loadTitle(new NUSDataProviderWUD(pPartition, pDrive), commonKey);
}
