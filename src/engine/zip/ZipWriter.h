#pragma once

#include "engine/Error.h"
#include "engine/io/FileSystem.h"

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine::zip {

class ZipWriter
{
public:
    static Result<ZipWriter> create(const std::filesystem::path &path);

    Result<void> add(std::string_view name, std::span<const std::uint8_t> data, int level = 9,
                     std::time_t modified = std::time(nullptr));
    Result<void> add(std::string_view name, std::string_view text, int level = 9,
                     std::time_t modified = std::time(nullptr))
    {
        return add(name, std::span(reinterpret_cast<const std::uint8_t *>(text.data()), text.size()), level,
                   modified);
    }

    Result<void> finish();

private:
    struct Entry
    {
        std::string name;
        std::uint32_t crc = 0;
        std::uint32_t compressedSize = 0;
        std::uint32_t size = 0;
        std::uint32_t offset = 0;
        std::uint16_t method = 0;
        std::uint16_t dosTime = 0;
        std::uint16_t dosDate = 0;
    };

    Result<void> write(std::span<const std::uint8_t> bytes);

    io::OutputFile file_;
    std::vector<Entry> entries_;
    std::uint64_t offset_ = 0;
};

}
