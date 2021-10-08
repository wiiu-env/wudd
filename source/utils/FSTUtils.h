#pragma once

#include <string>
#include <WUD/entities/FST/nodeentry/NodeEntry.h>
#include <WUD/entities/FST/nodeentry/DirectoryEntry.h>
#include <WUD/entities/FST/FST.h>

class FSTUtils{
public:
    static NodeEntry *getFSTEntryByFullPath(DirectoryEntry *root, std::string &givenFullPath);

    static DirectoryEntry *getFileEntryDir(DirectoryEntry *curEntry, std::string &string);

    static SectionEntry *getSectionEntryForIndex(FST *pFst, uint16_t index);
};

