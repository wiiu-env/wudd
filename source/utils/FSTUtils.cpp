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

#include "FSTUtils.h"
#include "logger.h"
#include <filesystem>

std::optional<std::shared_ptr<NodeEntry>> FSTUtils::getFSTEntryByFullPath(const std::shared_ptr<DirectoryEntry> &root, std::string &givenFullPath) {
    std::string fullPath = givenFullPath;
    if (strncmp(fullPath.c_str(), "/", 1) != 0) {
        fullPath = "/" + fullPath;
    }

    auto pathOpt                 = std::optional(root);
    std::filesystem::path asPath = fullPath;
    std::string dirPath          = asPath.parent_path().string();
    if (dirPath != "/") {
        pathOpt = getFileEntryDir(root, dirPath);
    }
    if (!pathOpt.has_value()) {
        return {};
    }
    for (auto &child : pathOpt.value()->getChildren()) {
        if (child->getFullPath() == fullPath) {
            return child;
        }
    }

    return {};
}

std::optional<std::shared_ptr<DirectoryEntry>> FSTUtils::getFileEntryDir(const std::shared_ptr<DirectoryEntry> &curEntry, std::string &string) {
    // We add the "/" at the end so we don't get false results when using the "startWith" function.
    if (!string.ends_with("/")) {
        string += "/";
    }
    for (auto &curChild : curEntry->getDirChildren()) {
        std::string compareTo = curChild->getFullPath();
        if (!compareTo.ends_with("/")) {
            compareTo += "/";
        }
        if (string.starts_with(compareTo)) {
            if (string == compareTo) {
                return curChild;
            }
            return getFileEntryDir(curChild, string);
        }
    }
    return {};
}

std::optional<std::shared_ptr<SectionEntry>> FSTUtils::getSectionEntryForIndex(const std::shared_ptr<FST> &pFst, uint16_t index) {
    for (const auto &entry : pFst->sectionEntries->getSections()) {
        if (entry->sectionNumber == index) {
            return entry;
        }
    }
    return {};
}
