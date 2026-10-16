#pragma once

#include "engine/Error.h"
#include "engine/crypto/Sha256.h"
#include "engine/io/RandomAccessFile.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace engine::p4k {

enum class Method : std::uint16_t {
    Stored = 0,
    Deflated = 8,
    Zstd = 100,
};

std::string_view methodName(std::uint16_t method);

inline constexpr std::array<std::uint8_t, 16> kCigKey = {0x5E, 0x7A, 0x20, 0x02, 0x30, 0x2E, 0xEB, 0x1A,
                                                         0x3B, 0xB6, 0x17, 0xC3, 0x0F, 0xDE, 0x1E, 0x47};

struct Entry
{
    std::uint64_t localHeaderOffset = 0;
    std::uint64_t compressedSize = 0;
    std::uint64_t uncompressedSize = 0;
    std::uint32_t nameOffset = 0;
    std::uint32_t centralOffset = 0;
    std::uint32_t crc32 = 0;
    std::uint32_t dosDateTime = 0;
    std::uint16_t nameLength = 0;
    std::uint16_t method = 0;
    std::uint16_t flags = 0;
    bool encrypted = false;
};

using Sink = std::function<Result<void>(std::span<const std::uint8_t>)>;
using ProgressFn = std::function<void(std::uint64_t done, std::uint64_t total)>;

struct OpenOptions
{
    ProgressFn progress;
    const std::atomic<bool> *cancel = nullptr;
    std::array<std::uint8_t, 16> key = kCigKey;
};

class Archive
{
public:
    static Result<std::shared_ptr<const Archive>> open(const std::filesystem::path &path,
                                                       const OpenOptions &options = {});

    const std::filesystem::path &path() const { return file_.path(); }
    std::uint64_t fileSize() const { return file_.size(); }

    std::size_t entryCount() const { return entries_.size(); }
    const Entry &entry(std::size_t index) const { return entries_[index]; }
    std::string_view name(std::size_t index) const;

    std::optional<std::size_t> find(std::string_view path) const;

    Result<void> read(std::size_t index, const Sink &sink) const;
    Result<std::vector<std::uint8_t>> read(std::size_t index) const;

    Result<void> readStored(std::size_t index, const Sink &sink) const;

    Result<std::optional<crypto::Sha256::Digest>> storedHash(std::size_t index) const;

private:
    Archive() = default;

    Result<void> parseDirectory(const OpenOptions &options);
    Result<std::uint64_t> dataOffset(const Entry &entry) const;
    void buildLookup() const;

    io::RandomAccessFile file_;
    std::vector<Entry> entries_;
    std::vector<char> names_;
    std::uint64_t centralDirectoryOffset_ = 0;
    std::array<std::uint8_t, 16> key_{};

    mutable std::once_flag lookupOnce_;
    mutable std::vector<std::uint32_t> lookup_;
};

}
