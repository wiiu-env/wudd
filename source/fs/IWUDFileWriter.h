#pragma once
#include <set>
#include <string>

#include <cstdint>

class IWUDFileWriter {
public:
    virtual ~IWUDFileWriter() = default;

    virtual int32_t writeSector(const uint8_t *sector_buf, size_t size) = 0;

    virtual bool finalize() = 0;

    [[nodiscard]] virtual std::set<std::string> getPaths() const = 0;

    [[nodiscard]] virtual bool isReady() const = 0;
};
