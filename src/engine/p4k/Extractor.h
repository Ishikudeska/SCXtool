#pragma once

#include "engine/Error.h"
#include "engine/Parallel.h"
#include "engine/p4k/Archive.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine::p4k {

bool unp4kFilterMatches(std::string_view filter, std::string_view name);

bool globMatches(std::string_view pattern, std::string_view name);

std::vector<std::size_t> selectEntries(const Archive &archive,
                                       const std::function<bool(std::string_view name)> &predicate);

struct ExtractOptions
{
    std::filesystem::path outputDir;
    bool skipExisting = false;
    unsigned threads = 0;
    ProgressCallback progress;
    const std::atomic<bool> *cancel = nullptr;
    std::function<Result<std::vector<std::uint8_t>>(std::string_view name, std::vector<std::uint8_t> bytes)>
        transform;
};

struct ExtractStats
{
    std::size_t extracted = 0;
    std::size_t skipped = 0;
    std::uint64_t bytes = 0;
    std::vector<std::string> failures;
};

Result<ExtractStats> extract(const Archive &archive, std::span<const std::size_t> entries,
                             const ExtractOptions &options);

std::filesystem::path entryOutputPath(const std::filesystem::path &outputDir, std::string_view name);

}
