#pragma once
#include "IWUDFileWriter.h"

class StubWudFileWriter : public IWUDFileWriter {
public:
    StubWudFileWriter()           = default;
    ~StubWudFileWriter() override = default;
    int32_t writeSector(const uint8_t *sector_buf, size_t size) override {
        return 0;
    }

    bool finalize() override {
        return true;
    }

    [[nodiscard]] std::set<std::string> getPaths() const override {
        return {};
    }

    [[nodiscard]] bool isReady() const override {
        return true;
    }
};
