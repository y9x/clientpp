#pragma once
#include "./IPCMessages.h"
#include <WebView2.h>
#include <rapidjson/document.h>
#include <rapidjson/fwd.h>
#include <string>
#include <wil/com.h>

class JSMessage {
public:
  rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator;
  unsigned short event;
  // std::vector<rapidjson::Value> args;
  rapidjson::Value args;
  JSMessage(const JSMessage &message);
  JSMessage(unsigned short event);
  JSMessage(unsigned short event, const rapidjson::Value &args);
  JSMessage(const std::string &raw);
  JSMessage();
  std::string dump();
  bool send(wil::com_ptr<ICoreWebView2> target);
  bool send(ICoreWebView2 *target);
};