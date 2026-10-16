#pragma once

#include <expected>
#include <string>
#include <utility>

namespace engine {

enum class Errc {
    Io,
    NotFound,
    Locked,
    Format,
    Unsupported,
    Corrupt,
    Cancelled,
};

struct Error
{
    Errc code = Errc::Io;
    std::string message;
};

template <class T> using Result = std::expected<T, Error>;

inline std::unexpected<Error> fail(Errc code, std::string message)
{
    return std::unexpected(Error{code, std::move(message)});
}

}
