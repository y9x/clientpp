#include "./JSMessage.h"
#include "../utils/StringUtil.h"

using namespace StringUtil;
using JSON = nlohmann::json;

JSMessage::JSMessage() {}

JSMessage::JSMessage(JSON a) { args = a; }

JSMessage::JSMessage(int e) { event = e; }

JSMessage::JSMessage(int e, JSON p) {
  event = e;
  args = p;
}

JSMessage::JSMessage(LPWSTR raw) {
  JSON parsed = JSON::parse(Convert::string(raw));

  if (!parsed.is_array())
    return;

  for (size_t index = 0; index < parsed.size(); index++) {
    if (index == 0) {
      if (!parsed[index].is_number())
        return;
      else
        event = parsed[index].get<int>();
    } else
      args.push_back(parsed[index]);
  }
}

std::string JSMessage::dump() {
  JSON message = JSON::array();

  message.push_back((int)event);
  for (JSON value : args)
    message.push_back(value);

  return message.dump();
}

bool JSMessage::send(ICoreWebView2 *target) {
  return SUCCEEDED(
      target->PostWebMessageAsJson(Convert::wstring(dump()).c_str()));
}

bool JSMessage::send(wil::com_ptr<ICoreWebView2> target) {
  return SUCCEEDED(
      target->PostWebMessageAsJson(Convert::wstring(dump()).c_str()));
}