
#include "FSTUtils.h"
#include "logger.h"
#include <filesystem>

NodeEntry *FSTUtils::getFSTEntryByFullPath(DirectoryEntry *root, std::string &givenFullPath) {
    std::string fullPath = givenFullPath;
    if (strncmp(fullPath.c_str(), "/", 1) != 0) {
        fullPath = "/" + fullPath;
    }

    DirectoryEntry *pathOpt = root;
    std::filesystem::path asPath = fullPath;
    std::string dirPath = asPath.parent_path().string();
    if (dirPath != "/") {
        pathOpt = getFileEntryDir(root, dirPath);
    }
    if (pathOpt == nullptr) {
        return nullptr;
    }
    for (auto &child: pathOpt->getChildren()) {
        if (child->getFullPath() == fullPath) {
            return child;
        }
    }

    return nullptr;
}

DirectoryEntry *FSTUtils::getFileEntryDir(DirectoryEntry *curEntry, std::string &string) {
    // We add the "/" at the end so we don't get false results when using the "startWith" function.
    if (!string.ends_with("/")) {
        string += "/";
    }
    for (auto &curChild: curEntry->getDirChildren()) {
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
    return nullptr;
}

SectionEntry *FSTUtils::getSectionEntryForIndex(FST *pFst, uint16_t index) {
    if (pFst == nullptr || pFst->sectionEntries == nullptr) {
        return nullptr;
    }

    for (const auto &entry: pFst->sectionEntries->getSections()) {
        if (entry->sectionNumber == index) {
            return entry;
        }
    }
    return nullptr;
}
