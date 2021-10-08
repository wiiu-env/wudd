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

TitleMetaData::TitleMetaData(uint8_t *data) {
    contentCount = ((uint16_t *) &data[0x1DE])[0];
    // Get Contents
    for (uint16_t i = 0; i < contentCount; i++) {
        auto curOffset = 0xB04 + (i * Content::LENGTH);
        auto *c = new Content((uint8_t *) &data[curOffset]);
        contentList.push_back(c);
    }
}

TitleMetaData::~TitleMetaData() {
    for (auto &content: contentList) {
        delete content;
    }
}

Content *TitleMetaData::getContentByIndex(uint16_t i) {
    for (auto &content: contentList) {
        if (content->index == i) {
            return content;
        }
    }
    return nullptr;
}
