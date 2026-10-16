#pragma once

#include "engine/Error.h"
#include "engine/forge/DataForge.h"
#include "engine/xml/XmlTree.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace engine::gamedata {

struct Options
{
    std::optional<std::string> channel;
    std::string baseIniPath;
    std::string overlayPath;
    std::string generatedAt;
};

using LogSink = std::function<void(const std::string &line)>;

Result<std::string> buildGameDataJson(const forge::DataForge &forge, const Options &options,
                                      const LogSink &log = {});

struct RecordSource
{
    std::vector<std::string> paths;
    std::function<xml::XmlTree::NodeId(std::size_t, xml::XmlTree &)> build;
    int dcbVersion = 0;
};
Result<std::string> buildGameDataJson(const RecordSource &records, const Options &options,
                                      const LogSink &log = {});

}
