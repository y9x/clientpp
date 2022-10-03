#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include "./JSMessage.h"
#include "../utils/StringUtil.h"
#include "./Log.h"

using namespace StringUtil;

JSMessage::JSMessage() : args(rapidjson::kArrayType) {}

// JSMessage::JSMessage(rapidjson::Value a) : rapidjson::Document() { args = a; }

JSMessage::JSMessage(int e) : args(rapidjson::kArrayType) { event = e; }

JSMessage::JSMessage(int e, rapidjson::Value p) : event(e), args(p, allocator)
{
}

JSMessage::JSMessage(const JSMessage &message) : event(message.event), args(message.args, allocator) {}

JSMessage::JSMessage(LPWSTR raw) : args(rapidjson::kArrayType)
{
  std::string str = Convert::string(raw);

  rapidjson::Document document;

  rapidjson::ParseResult ok = document.Parse(str.data(), str.size());

  if (!ok)
    clog::error << "Error parsing message: " << GetParseError_En(ok.Code()) << " (" << ok.Offset() << ")" << clog::endl;

  if (!document.IsArray())
  {
    clog::error << "Message was not array" << std::endl;
    return;
  }

  rapidjson::Value::ValueIterator it = document.Begin();

  if (it == document.End())
  {
    clog::error << "Message had no elements" << std::endl;
    return;
  }

  rapidjson::Value &e = *it;

  it++;

  if (!e.IsNumber())
  {
    clog::error << "First element was not number" << std::endl;
    return;
  }

  event = e.GetInt();

  for (; it != document.End(); ++it)
  {
    args.PushBack(rapidjson::Value(*it, allocator), allocator);
  }
}

std::string JSMessage::dump()
{
  rapidjson::Document message(rapidjson::kArrayType);

  message.PushBack(rapidjson::Value(event), message.GetAllocator());

  for (rapidjson::Value::ValueIterator it = args.Begin(); it != args.End(); ++it)
  {
    message.PushBack(*it, message.GetAllocator());
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  message.Accept(writer);

  return {buffer.GetString(), buffer.GetSize()};
}

bool JSMessage::send(ICoreWebView2 *target)
{
  return SUCCEEDED(
      target->PostWebMessageAsJson(Convert::wstring(dump()).c_str()));
}

bool JSMessage::send(wil::com_ptr<ICoreWebView2> target)
{
  return SUCCEEDED(
      target->PostWebMessageAsJson(Convert::wstring(dump()).c_str()));
}