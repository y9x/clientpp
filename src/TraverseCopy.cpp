#include "./TraverseCopy.h"
#include "./Log.h"
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/allocators.h>

rapidjson::Value TraverseCopy(rapidjson::Value &value, rapidjson::Value &match, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator, bool allow_new_props, bool *changed)
{
  rapidjson::Value result;

  if (value.GetType() != match.GetType())
  {
    if (changed)
      *changed = true;
    result.CopyFrom(match, allocator);
    return result;
  }

  if (value.IsObject())
  {
    // (rapidjson::kNullType);

    result.CopyFrom(match, allocator);

    for (rapidjson::Value::MemberIterator it = value.MemberBegin(); it != value.MemberEnd(); ++it)
    {
      // std::string key(it->name.GetString(), it->name.GetStringLength());
      // rapidjson::Value skey(key.data(), key.size());

      if (match.HasMember(it->name))
        result[it->name] =
            TraverseCopy(it->value, match[it->name], allocator, allow_new_props, changed);
      else if (allow_new_props && !result.HasMember(it->name))
        result.AddMember(it->name, it->value, allocator);
      else if (changed)
        *changed = true;
    }

    return result;
  }
  else
  {
    result.CopyFrom(value, allocator);
    return result;
  }
}