#pragma once

#include "engine/Error.h"

#include <cstdint>
#include <filesystem>
#include <span>

namespace engine::io {

class RandomAccessFile
{
public:
    RandomAccessFile() = default;
    RandomAccessFile(RandomAccessFile &&other) noexcept;
    RandomAccessFile &operator=(RandomAccessFile &&other) noexcept;
    RandomAccessFile(const RandomAccessFile &) = delete;
    RandomAccessFile &operator=(const RandomAccessFile &) = delete;
    ~RandomAccessFile();

    static Result<RandomAccessFile> open(const std::filesystem::path &path);

    bool isOpen() const;
    std::uint64_t size() const { return size_; }
    const std::filesystem::path &path() const { return path_; }

    Result<void> readAt(std::uint64_t offset, std::span<std::uint8_t> buffer) const;

private:
    void close();

#ifdef _WIN32
    void *handle_ = nullptr;
#else
    int fd_ = -1;
#endif
    std::uint64_t size_ = 0;
    std::filesystem::path path_;
};

}
