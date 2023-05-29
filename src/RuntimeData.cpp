#include "../utils/IOUtil.h"
#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "./ChScriptedWindow.h"
#include "./TraverseCopy.h"
#include "LoadRes.h"
#include "Log.h"
#include "resource.h"
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

rapidjson::Value ChScriptedWindow::getUserScripts(
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator) {
  rapidjson::Value result(rapidjson::kArrayType);

  for (auto const &dirEntry : std::filesystem::directory_iterator(
           folder.directory + folder.p_scripts)) {
    std::string code;
    if (IOUtil::readFile(dirEntry.path(), code)) {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string script = dirEntry.path().string();
      std::string scriptID = dirEntry.path().filename().string();

      // [script: string, scriptID: string, code: string]
      row.PushBack(rapidjson::Value(script.data(), script.size(), allocator),
                   allocator);
      row.PushBack(
          rapidjson::Value(scriptID.data(), scriptID.size(), allocator),
          allocator);
      row.PushBack(rapidjson::Value(code.data(), code.size(), allocator),
                   allocator);
      result.PushBack(row, allocator);
    } else
      clog::error << "Failure reading userscript " << dirEntry << clog::endl;
  }

  return result;
}

rapidjson::Value ChScriptedWindow::getUserStyles(
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator) {
  rapidjson::Value result(rapidjson::kArrayType);

  for (auto const &dirEntry : std::filesystem::directory_iterator(
           folder.directory + folder.p_styles)) {
    std::string code;
    if (IOUtil::readFile(dirEntry.path(), code)) {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string script = dirEntry.path().string();
      // [script: string, code: string]
      row.PushBack(rapidjson::Value(script.data(), script.size(), allocator),
                   allocator);
      row.PushBack(rapidjson::Value(code.data(), code.size(), allocator),
                   allocator);
      result.PushBack(row, allocator);
    } else
      clog::error << "Failure reading userscript " << dirEntry << clog::endl;
  }

  return result;
}

std::string ChScriptedWindow::runtimeData() {
  rapidjson::Document data(rapidjson::kObjectType);
  rapidjson::Document::AllocatorType allocator = data.GetAllocator();

  data.AddMember("config", rapidjson::Value(folder.config, allocator),
                 allocator);
  data.AddMember("css", getUserStyles(allocator), allocator);
  data.AddMember("js", getUserScripts(allocator), allocator);
  data.AddMember("version",
                 rapidjson::Value(CLIENT_VERSION_STRING, data.GetAllocator()),
                 allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  data.Accept(writer);

  return {buffer.GetString(), buffer.GetSize()};
}