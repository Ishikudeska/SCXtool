#pragma once

#include "engine/Error.h"
#include "engine/forge/DataForge.h"
#include "engine/forge/RecordBuilder.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace engine::forge {

struct ExportOptions
{
    BuildOptions build;
    std::function<bool(std::string_view path)> include;
    unsigned threads = 0;
    std::function<void(std::size_t done, std::size_t total)> progress;
    const std::atomic<bool> *cancel = nullptr;
};

struct ExportStats
{
    std::size_t written = 0;
    std::size_t skipped = 0;
    std::uint64_t bytes = 0;
    std::vector<std::string> failures;
};

std::vector<std::uint32_t> selectRecords(const DataForge &forge,
                                         const std::function<bool(std::string_view)> &include);

Result<ExportStats> exportRecords(const DataForge &forge, const std::filesystem::path &outRoot,
                                  const ExportOptions &options);

std::filesystem::path recordOutputPath(const std::filesystem::path &outRoot, std::string_view recordPath);

}
