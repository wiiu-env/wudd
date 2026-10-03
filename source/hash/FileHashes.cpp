#include "FileHashes.h"

#include <coreinit/cache.h>

#include <utility>

#include <cstring>

FileHashes::FileHashes() {
    reset();
}

FileHashes::~FileHashes() = default;

void FileHashes::reset() {
    mDigestCrc32.reset();
    mDigestMd5.reset();
    mDigestSha1.reset();
    mDigestSha2.reset();

    mHashThread = std::jthread(&FileHashes::hashWorker, this);
    OSMemoryBarrier();
}

// Background worker loop
void FileHashes::hashWorker(const std::stop_token &stoken) {
    while (true) {
        HashJob job;
        {
            std::unique_lock lock(mQueueMutex);

            // Wait until there is work in the queue OR a stop is requested.
            // cv.wait returns false if a stop was requested AND the predicate (queue not empty) is false.
            const bool hasWork = mQueueCv.wait(lock, stoken, [this] { return !mHashQueue.empty(); });

            if (!hasWork) {
                // Thread was asked to stop, and the queue is completely empty. We are done.
                break;
            }

            // Grab the job and remove it from the queue
            job = std::move(mHashQueue.front());
            mHashQueue.pop();
        }

        mDigestCrc32.add(job.buffer.get(), job.size);
        mDigestMd5.add(job.buffer.get(), job.size);
        mDigestSha1.add(job.buffer.get(), job.size);
        mDigestSha2.add(job.buffer.get(), job.size);
        OSMemoryBarrier();
    }
}

void FileHashes::processHashes(const uint8_t *buffer, uint32_t bufferSize) {
    auto memory_cpy = std::make_unique<uint8_t[]>(bufferSize);
    memcpy(memory_cpy.get(), buffer, bufferSize);
    {
        std::lock_guard lock(mQueueMutex);
        mHashQueue.push(HashJob{std::move(memory_cpy), bufferSize});
        OSMemoryBarrier();
    }
    mQueueCv.notify_one();
}

void FileHashes::finalize() {
    mHashThread.request_stop();

    if (mHashThread.joinable()) {
        mHashThread.join();
    }

    mHashCRC32Opt = mDigestCrc32.getHash();
    mHashMD5Opt   = mDigestMd5.getHash();
    mHashSHA1Opt  = mDigestSha1.getHash();
    mHashSHA2Opt  = mDigestSha2.getHash();
}

std::map<FileHashes::HashType, std::string> FileHashes::getHashes() {
    std::map<HashType, std::string> result;
    if (mHashCRC32Opt) {
        result[HashType::CRC32] = mDigestCrc32.getHash();
    }
    if (mHashMD5Opt) {
        result[HashType::MD5] = mDigestMd5.getHash();
    }
    if (mHashSHA1Opt) {
        result[HashType::SHA1] = mDigestSha1.getHash();
    }
    if (mHashSHA2Opt) {
        result[HashType::SHA256] = mDigestSha2.getHash();
    }
    return result;
}