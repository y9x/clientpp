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

  for (auto const &dir_entry : std::filesystem::directory_iterator(
           folder.directory + folder.p_scripts)) {
    std::string buffer;

    if (IOUtil::read_file(dir_entry.path(), buffer)) {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string name = dir_entry.path().string();
      row.PushBack(rapidjson::Value(name.data(), name.size(), allocator),
                   allocator);
      row.PushBack(rapidjson::Value(buffer.data(), buffer.size(), allocator),
                   allocator);
      result.PushBack(row, allocator);
    } else
      clog::error << "Failure reading userscript " << dir_entry << clog::endl;
  }

  return result;
}

rapidjson::Value ChScriptedWindow::getUserStyles(
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator) {
  rapidjson::Value result(rapidjson::kArrayType);

  for (auto const &dir_entry : std::filesystem::directory_iterator(
           folder.directory + folder.p_styles)) {
    std::string buffer;

    if (IOUtil::read_file(dir_entry.path(), buffer)) {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string name = dir_entry.path().string();
      row.PushBack(rapidjson::Value(name.data(), name.size(), allocator),
                   allocator);
      row.PushBack(rapidjson::Value(buffer.data(), buffer.size(), allocator),
                   allocator);
      result.PushBack(row, allocator);
    } else
      clog::error << "Failure reading userscript " << dir_entry << clog::endl;
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