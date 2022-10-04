#include <rapidjson/document.h>
#include <rapidjson/allocators.h>
#include <rapidjson/writer.h>
#include "./TraverseCopy.h"
#include "./Log.h"

rapidjson::Value TraverseCopy(rapidjson::Value &value, rapidjson::Value &match, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator, bool allow_new_props, bool *changed)
{
  rapidjson::Value result;

  // there is only a true or false type, no bool
  if (value.IsBool() != match.IsBool() && value.GetType() != match.GetType())
  {
    if (changed)
      *changed = true;

    result.CopyFrom(match, allocator);
  }
  else if (value.IsObject())
  {
    result.CopyFrom(match, allocator);

    for (rapidjson::Value::MemberIterator it = value.MemberBegin(); it != value.MemberEnd(); ++it)
      if (match.HasMember(it->name))
        result[it->name] =
            TraverseCopy(it->value, match[it->name], allocator, allow_new_props, changed);
      else if (allow_new_props)
        result.AddMember(it->name, it->value, allocator);
      else if (changed)
        *changed = true;
  }
  else
    result.CopyFrom(value, allocator);

  return result;
}