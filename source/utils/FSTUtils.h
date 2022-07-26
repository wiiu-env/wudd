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

#include <WUD/entities/FST/FST.h>
#include <WUD/entities/FST/nodeentry/DirectoryEntry.h>
#include <WUD/entities/FST/nodeentry/NodeEntry.h>
#include <string>

class FSTUtils {
public:
    static std::optional<std::shared_ptr<NodeEntry>> getFSTEntryByFullPath(const std::shared_ptr<DirectoryEntry> &root, std::string &givenFullPath);

    static std::optional<std::shared_ptr<DirectoryEntry>> getFileEntryDir(const std::shared_ptr<DirectoryEntry> &curEntry, std::string &string);

    static std::optional<std::shared_ptr<SectionEntry>> getSectionEntryForIndex(const std::shared_ptr<FST> &pFst, uint16_t index);
};
