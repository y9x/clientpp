#pragma once
#include "./IPCMessages.h"
#include <rapidjson/allocators.h>
#include <rapidjson/fwd.h>
#include <rapidjson/pointer.h>
#include <string>

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
  std::string dump() const;
};