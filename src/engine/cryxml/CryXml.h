#pragma once

#include "engine/Error.h"
#include "engine/xml/XmlTree.h"

#include <cstdint>
#include <span>
#include <string>

namespace engine::cryxml {

bool isCryXml(std::span<const std::uint8_t> data);

bool isPlainXml(std::span<const std::uint8_t> data);

Result<xml::XmlTree::NodeId> parse(std::span<const std::uint8_t> data, xml::XmlTree &tree);

Result<std::string> toXml(std::span<const std::uint8_t> data);

}
