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
#pragma once

#include <WUD/NUSDataProvider.h>
#include <WUD/entities/TMD/TitleMetaData.h>
#include <WUD/content/partitions/WiiUGMPartition.h>
#include "Ticket.h"
#include "NUSDataProcessor.h"
#include "NUSDecryption.h"
#include "DefaultNUSDataProcessor.h"
#include "DiscReaderDiscDrive.h"
#include "NUSDataProviderWUD.h"

class NUSTitle {

public:
    ~NUSTitle();

    NUSDataProcessor *dataProcessor;
    TitleMetaData *tmd;
    Ticket *ticket;
    FST *fst;
    NUSDecryption *decryption;
    NUSDataProvider *dataProvider;

    static NUSTitle *loadTitleFromGMPartition(WiiUGMPartition *pPartition, DiscReaderDiscDrive *pDrive, uint8_t commonKey[16]);

private:
    static NUSTitle *loadTitle(NUSDataProvider *dataProvider, uint8_t commonKey[16]);

    NUSTitle(TitleMetaData *pTMD, NUSDataProcessor *pProcessor, NUSDataProvider *pDataProvider, NUSDecryption *pDecryption, Ticket *pTicket, FST *pFST);

};