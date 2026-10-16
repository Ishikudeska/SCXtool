#pragma once

#include <string>

namespace engine {

struct LibraryVersions
{
    std::string zstd;
    std::string zlib;
    std::string pugixml;
};

LibraryVersions libraryVersions();

}
