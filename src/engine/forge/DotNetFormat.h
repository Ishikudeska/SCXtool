#pragma once

#include <cstdint>
#include <string>

namespace engine::forge {

void appendSingle(std::string &out, float value);
void appendDouble(std::string &out, double value);

std::string formatSingle(float value);
std::string formatDouble(double value);

void appendGuid(std::string &out, const std::uint8_t *bytes);
std::string formatGuid(const std::uint8_t *bytes);

}
