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
#include "TitleMetaData.h"

#include <utility>
#include <utils/logger.h>

TitleMetaData::TitleMetaData(uint64_t titleId, std::vector<std::shared_ptr<Content>> pContentList) : contentList(std::move(pContentList)) {
    this->titleId = titleId;
}

std::optional<std::shared_ptr<Content>> TitleMetaData::getContentByIndex(uint16_t i) {
    for (auto &content : contentList) {
        if (content->index == i) {
            return content;
        }
    }
    return {};
}

std::optional<std::unique_ptr<TitleMetaData>> TitleMetaData::make_unique(const std::vector<uint8_t> &data) {
    if (data.empty() || data.size() <= 0xB04) {
        return {};
    }
    std::vector<std::shared_ptr<Content>> contentList;
    auto contentCount = ((uint16_t *) &data[0x1DE])[0];
    auto titleID      = ((uint64_t *) &data[0x18C])[0];
    for (uint16_t i = 0; i < contentCount; i++) {
        auto curOffset = 0xB04 + (i * Content::LENGTH);
        if (data.size() < curOffset + Content::LENGTH) {
            DEBUG_FUNCTION_LINE_ERR("Failed to parse TitleMetaData");
            return {};
        }
        std::array<uint8_t, Content::LENGTH> contentData{};
        std::copy(data.begin() + (int) curOffset, data.begin() + (int) curOffset + Content::LENGTH, contentData.begin());

        auto curContentOpt = Content::make_shared(contentData);
        if (!curContentOpt.has_value()) {
            DEBUG_FUNCTION_LINE_ERR("Failed to parse Content");
            return {};
        }
        contentList.push_back(curContentOpt.value());
    }

    return std::unique_ptr<TitleMetaData>(new TitleMetaData(titleID, contentList));
}
