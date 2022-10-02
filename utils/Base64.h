#pragma once
#include <string>

namespace Base64 {
std::string Encode(const std::string &decoded);
std::string Decode(const std::string &encoded);
}; // namespace Base64