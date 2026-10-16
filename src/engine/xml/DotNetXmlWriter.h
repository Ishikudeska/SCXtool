#pragma once

#include "engine/xml/XmlTree.h"

#include <optional>
#include <string>
#include <string_view>

namespace engine::xml {

void writeDotNet(const XmlTree &tree, XmlTree::NodeId root, std::string &out);

std::optional<std::string> dotNetNameError(std::string_view latin1);

void appendLatin1AsUtf8(std::string &out, std::string_view latin1);

}
