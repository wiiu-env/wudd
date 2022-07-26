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

#include "DefaultNUSDataProcessor.h"
#include "DiscReaderDiscDrive.h"
#include "NUSDataProcessor.h"
#include "NUSDataProviderWUD.h"
#include "NUSDecryption.h"
#include "Ticket.h"
#include "utils/StringTools.h"
#include <WUD/NUSDataProvider.h>
#include <WUD/content/partitions/WiiUGMPartition.h>
#include <WUD/entities/TMD/TitleMetaData.h>
#include <nn/acp/title.h>

class NUSTitle {

public:
    ~NUSTitle();

    std::unique_ptr<NUSDataProcessor> dataProcessor;
    std::unique_ptr<TitleMetaData> tmd;
    std::unique_ptr<Ticket> ticket;
    std::shared_ptr<FST> fst;
    std::shared_ptr<NUSDecryption> decryption;
    std::shared_ptr<NUSDataProvider> dataProvider;

    static std::optional<std::unique_ptr<NUSTitle>> loadTitleFromGMPartition(
            std::shared_ptr<WiiUGMPartition> pPartition,
            std::shared_ptr<DiscReader> pDiscReader,
            const std::array<uint8_t, 16> &commonKey);

    std::string getLongnameEn();

    std::string getShortnameEn();

private:
    std::optional<std::string> longname_en;
    std::optional<std::string> shortname_en;

    static std::optional<std::unique_ptr<NUSTitle>> make_unique(std::unique_ptr<NUSDataProvider> dataProvider, const std::array<uint8_t, 16> &commonKey);

    NUSTitle(std::unique_ptr<TitleMetaData> pTMD,
             std::unique_ptr<NUSDataProcessor> pProcessor,
             std::shared_ptr<NUSDataProvider> pDataProvider,
             std::shared_ptr<NUSDecryption> pDecryption,
             std::unique_ptr<Ticket> pTicket,
             std::shared_ptr<FST> pFST);
};