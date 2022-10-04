#pragma once
#include <string>
#include <vector>

namespace ST
{
    std::wstring wstring(const std::string &str);
    std::string string(const std::wstring &str);

    std::string lowercase(const std::string &string);
    std::wstring lowercase(const std::wstring &string);

    std::string uppercase(const std::string &string);
    std::wstring uppercase(const std::wstring &string);

    std::string replace_all(std::string string, const std::string &from,
                            const std::string &to);
    std::wstring replace_all(std::wstring string, const std::wstring &from,
                             const std::wstring &to);
}; // namespace ST