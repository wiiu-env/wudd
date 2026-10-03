#pragma once
#include "crc32.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

class FileHashes {
public:
    FileHashes();
    ~FileHashes();
    void reset();
    void processHashes(const uint8_t *buffer, uint32_t bufferSize);
    void finalize();

    enum class HashType {
        CRC32,
        MD5,
        SHA1,
        SHA256,
    };

    std::map<HashType, std::string> getHashes();

private:
    void hashWorker(const std::stop_token &stoken);

    CRC32 mDigestCrc32;
    MD5 mDigestMd5;
    SHA1 mDigestSha1;
    SHA256 mDigestSha2;

    std::optional<std::string> mHashCRC32Opt;
    std::optional<std::string> mHashMD5Opt;
    std::optional<std::string> mHashSHA1Opt;
    std::optional<std::string> mHashSHA2Opt;

    struct HashJob {
        std::unique_ptr<uint8_t[]> buffer;
        uint32_t size;
    };

    std::queue<HashJob> mHashQueue;
    std::recursive_mutex mQueueMutex;
    std::condition_variable_any mQueueCv;
    std::jthread mHashThread;
};
