#pragma once

#include "engine/Error.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace engine::io {

#ifdef _WIN32
std::wstring longPath(const std::filesystem::path &path);

std::string systemErrorMessage(unsigned long code);
#endif

bool exists(const std::filesystem::path &path);

Result<void> forEachFile(const std::filesystem::path &root,
                         const std::function<void(std::string_view relativePath)> &visit);

Result<void> removeAll(const std::filesystem::path &path);

Result<void> createDirectories(const std::filesystem::path &dir);

class OutputFile
{
public:
    OutputFile() = default;
    OutputFile(OutputFile &&other) noexcept;
    OutputFile &operator=(OutputFile &&other) noexcept;
    OutputFile(const OutputFile &) = delete;
    OutputFile &operator=(const OutputFile &) = delete;
    ~OutputFile();

    static Result<OutputFile> create(const std::filesystem::path &target, bool atomic = true);

    Result<void> write(std::span<const std::uint8_t> bytes);
    Result<void> commit();

private:
    void discard();

#ifdef _WIN32
    void *handle_ = nullptr;
#else
    int fd_ = -1;
#endif
    std::filesystem::path target_;
    std::filesystem::path writingTo_;
};

Result<void> writeFile(const std::filesystem::path &target, std::span<const std::uint8_t> bytes,
                       bool atomic = true);

inline Result<void> writeFile(const std::filesystem::path &target, std::string_view text, bool atomic = true)
{
    return writeFile(target, std::span(reinterpret_cast<const std::uint8_t *>(text.data()), text.size()),
                     atomic);
}

}
