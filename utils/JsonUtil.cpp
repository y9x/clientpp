#include "./StringUtil.h"
#include "./JsonUtil.h"
#include <rapidjson/pointer.h>
#include <rapidjson/allocators.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace JsonUtil
{
    namespace Convert
    {
        std::wstring wstring(const rapidjson::Value &str)
        {
            return StringUtil::Convert::wstring(string(str));
        }

        std::string string(const rapidjson::Value &str)
        {
            return std::string(str.GetString(), str.GetStringLength());
        }
    }; // namespace Convert
};     // namespace JsonUtil