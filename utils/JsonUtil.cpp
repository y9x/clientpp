#include "./StringUtil.h"
#include "./JsonUtil.h"
#include <rapidjson/pointer.h>
#include <rapidjson/allocators.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace JT
{
    std::wstring wstring(const rapidjson::Value &str)
    {
        return ST::wstring(string(str));
    }

    std::string string(const rapidjson::Value &str)
    {
        return std::string(str.GetString(), str.GetStringLength());
    }
}; // namespace JT