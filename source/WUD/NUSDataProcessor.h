#pragma once

#include "NUSDataProvider.h"
#include "NUSDecryption.h"

class NUSDataProcessor {
protected:
    NUSDataProcessor() = default;

public:
    virtual ~NUSDataProcessor() = default;

    virtual std::shared_ptr<NUSDataProvider> &getDataProvider() = 0;

    virtual bool readPlainDecryptedContent(const std::shared_ptr<Content> &pContent, std::vector<uint8_t> &out_data) = 0;
};