#include "WUDUtils.h"
#include "FSTUtils.h"

std::optional<uint64_t> WUDUtils::getOffsetOfContent(const std::shared_ptr<WiiUGMPartition> &gamePartition, const std::shared_ptr<FST> &fst, const std::shared_ptr<Content> &content) {
    if (content->index == 0) { // Index 0 is the FST which is at the beginning of the partition;
        auto &vh = gamePartition->getVolumes().begin()->second;
        return gamePartition->getSectionOffsetOnDefaultPartition() + vh->FSTAddress.getAddressInBytes();
    }
    auto info = FSTUtils::getSectionEntryForIndex(fst, content->index);
    if (!info.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get section for Content");
        return {};
    }
    return gamePartition->getSectionOffsetOnDefaultPartition() + info.value()->address.getAddressInBytes();
}