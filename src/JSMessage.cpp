#include "./JSMessage.h"
#include "../utils/StringUtil.h"
#include "./Log.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>

constexpr short BAD_EVENT = 0xFFFF;

JSMessage::JSMessage() : args(rapidjson::kArrayType), event(BAD_EVENT) {}

// JSMessage::JSMessage(rapidjson::Value a) : args(a, allocator) {  }

JSMessage::JSMessage(unsigned short e)
    : args(rapidjson::kArrayType), event(e) {}

JSMessage::JSMessage(unsigned short e, const rapidjson::Value &p)
    : args(p, allocator), event(e) {}

JSMessage::JSMessage(const JSMessage &message)
    : event(message.event), args(message.args, allocator) {}

JSMessage::JSMessage(const std::string &raw)
    : args(rapidjson::kArrayType), event(BAD_EVENT) {
  rapidjson::Document document;

  rapidjson::ParseResult ok = document.Parse(raw.data(), raw.size());

  if (!ok)
    clog::error << "Error parsing message: " << GetParseError_En(ok.Code())
                << " (" << ok.Offset() << ")" << clog::endl;

  if (!document.IsArray()) {
    clog::error << "Message was not array" << std::endl;
    return;
  }

  rapidjson::Value::ValueIterator it = document.Begin();

  if (it == document.End()) {
    clog::error << "Message had no elements" << std::endl;
    return;
  }

  rapidjson::Value &e = *it;

  it++;

  if (!e.IsNumber()) {
    clog::error << "First element was not number" << std::endl;
    return;
  }

  event = static_cast<unsigned short>(e.GetUint());

  for (; it != document.End(); ++it) {
    args.PushBack(rapidjson::Value(*it, allocator), allocator);
  }
}

std::string JSMessage::dump() const {
  rapidjson::Document message(rapidjson::kArrayType);

  message.PushBack(rapidjson::Value(static_cast<unsigned int>(event)),
                   message.GetAllocator());

  for (rapidjson::Value::ConstValueIterator it = args.Begin(); it != args.End();
       ++it) {
    message.PushBack(rapidjson::Value(*it, message.GetAllocator()),
                     message.GetAllocator());
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  message.Accept(writer);

  return {buffer.GetString(), buffer.GetSize()};
}
