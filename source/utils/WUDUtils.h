#pragma once
#include "WUD/content/partitions/WiiUGMPartition.h"
#include "WUD/entities/FST/FST.h"
#include "WUD/entities/TMD/Content.h"
#include <memory>
#include <optional>

class WUDUtils {
public:
    static std::optional<uint64_t> getOffsetOfContent(const std::shared_ptr<WiiUGMPartition> &gamePartition, const std::shared_ptr<FST> &fst, const std::shared_ptr<Content> &content);
};
