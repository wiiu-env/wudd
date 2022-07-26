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

#include "NUSDataProcessor.h"
#include <utils/utils.h>

class DefaultNUSDataProcessor : public NUSDataProcessor {
public:
    DefaultNUSDataProcessor(std::shared_ptr<NUSDataProvider> pDataProvider, std::shared_ptr<NUSDecryption> pNUSDecryption);

    ~DefaultNUSDataProcessor() override = default;

    bool readPlainDecryptedContent(const std::shared_ptr<Content> &pContent, std::vector<uint8_t> &out_data) override;

    std::shared_ptr<NUSDataProvider> &getDataProvider() override;

private:
    std::shared_ptr<NUSDataProvider> dataProvider;
    std::shared_ptr<NUSDecryption> nusDecryption;
};
