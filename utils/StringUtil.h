#pragma once
#include <string>
#include <vector>

namespace StringUtil {
namespace Convert {
std::wstring wstring(std::string str);

std::string string(std::wstring str);
}; // namespace Convert

namespace Manipulate {
std::string slice(std::string string, int start, int end);
std::string slice(std::string string, int start);

std::string lowercase(std::string string);
std::string uppercase(std::string string);

std::string replace_all(std::string string, const std::string &from,
                        const std::string &to);
std::wstring replace_all(std::wstring string, const std::wstring &from,
                         const std::wstring &to);

std::vector<std::string> split(const std::string &str, char delim);
}; // namespace Manipulate

namespace Validate {
bool Number(std::string str);
}
}; // namespace StringUtil