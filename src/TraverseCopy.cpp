#include <rapidjson/document.h>
#include <rapidjson/allocators.h>
#include <rapidjson/writer.h>
#include "./TraverseCopy.h"
#include "./Log.h"

std::string dumpJson(const rapidjson::Value &value)
{
  rapidjson::Document doc;
  doc.CopyFrom(value, doc.GetAllocator());
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer>
      writer(buffer);
  doc.Accept(writer);
  return {buffer.GetString(), buffer.GetSize()};
}

rapidjson::Value TraverseCopy(rapidjson::Value &value, rapidjson::Value &match, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator, bool allow_new_props, bool *changed)
{
  rapidjson::Value result;

  if (value.GetType() != match.GetType())
  {
    if (changed)
      *changed = true;

    std::cout << dumpJson(value) << " did not eq " << dumpJson(match) << std::endl;

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