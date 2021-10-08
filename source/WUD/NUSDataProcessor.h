#pragma once

#include "NUSDataProvider.h"
#include "NUSDecryption.h"

class NUSDataProcessor {
protected:
    NUSDataProcessor() = default;

public:
    virtual ~NUSDataProcessor() = default;

    virtual NUSDataProvider *getDataProvider() = 0;

    virtual bool readPlainDecryptedContent(Content *pContent, uint8_t **data, uint32_t *length) = 0;
};