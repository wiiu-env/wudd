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

    std::shared_ptr<NUSDataProcessor> dataProcessor;
    std::shared_ptr<TitleMetaData> tmd;
    std::shared_ptr<Ticket> ticket;
    std::shared_ptr<FST> fst;
    std::shared_ptr<NUSDecryption> decryption;
    std::shared_ptr<NUSDataProvider> dataProvider;

    static std::optional<std::shared_ptr<NUSTitle>> loadTitleFromGMPartition(
            const std::shared_ptr<WiiUGMPartition> &pPartition,
            const std::shared_ptr<DiscReader> &pDrive,
            const std::array<uint8_t, 16> &commonKey);

private:
    static std::optional<std::shared_ptr<NUSTitle>> loadTitle(const std::shared_ptr<NUSDataProvider> &dataProvider, const std::array<uint8_t, 16> &commonKey);

    NUSTitle(std::shared_ptr<TitleMetaData> pTMD,
             std::shared_ptr<NUSDataProcessor> pProcessor,
             std::shared_ptr<NUSDataProvider> pDataProvider,
             std::shared_ptr<NUSDecryption> pDecryption,
             std::shared_ptr<Ticket> pTicket,
             std::shared_ptr<FST> pFST);

};