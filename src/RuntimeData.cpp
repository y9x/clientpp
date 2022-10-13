#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "./KrunkerWindow.h"
#include "./TraverseCopy.h"
#include "LoadRes.h"
#include "Log.h"
#include "resource.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

rapidjson::Value ChScriptedWindow::getUserScripts(
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator) {
  rapidjson::Value result(rapidjson::kArrayType);

  for (IOUtil::WDirectoryIterator it(folder.directory + folder.p_scripts,
                                     L"*.js");
       ++it;) {
    std::string buffer;

    if (IOUtil::read_file(it.path(), buffer)) {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string name = ST::string(it.path());
      row.PushBack(rapidjson::Value(name.data(), name.size(), allocator),
                   allocator);
      row.PushBack(rapidjson::Value(buffer.data(), buffer.size(), allocator),
                   allocator);
      result.PushBack(row, allocator);
    } else {
      clog::error << "Failure reading userscript " << ST::string(it.path())
                  << clog::endl;
    }
  }

  return result;
}

rapidjson::Value ChScriptedWindow::getUserStyles(
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator) {
  rapidjson::Value result(rapidjson::kArrayType);

  for (IOUtil::WDirectoryIterator it(folder.directory + folder.p_styles,
                                     L"*.css");
       ++it;) {
    std::string buffer;

    if (IOUtil::read_file(it.path().c_str(), buffer)) {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string name = ST::string(it.path());
      row.PushBack(rapidjson::Value(name.data(), name.size(), allocator),
                   allocator);
      row.PushBack(rapidjson::Value(buffer.data(), buffer.size(), allocator),
                   allocator);
      result.PushBack(row, allocator);
    }
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

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  data.Accept(writer);

  return {buffer.GetString(), buffer.GetSize()};
}