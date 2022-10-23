#define _CRT_SECURE_NO_WARNINGS
#include "./ClientFolder.h"
#include "../utils/IOUtil.h"
#include "../utils/StringUtil.h"
#include "./LoadRes.h"
#include "./Log.h"
#include "./TraverseCopy.h"
#include "resource.h"
#include <ShlObj_core.h>
#include <Windows.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>

// true if the result of CreateDirectory is nonzero or if GetLastError equals
// ERROR_ALREADY_EXISTS, otherwise false
bool OVR(int result) {
  if (result != 0)
    return true;
  else if (GetLastError() == ERROR_ALREADY_EXISTS)
    return true;
  else
    return false;
}

bool writeResource(std::wstring path, int resource) {
  HRSRC src = FindResource(NULL, MAKEINTRESOURCE(resource), RT_RCDATA);
  bool ret = false;

  if (src != NULL) {
    HGLOBAL header = LoadResource(NULL, src);
    if (header != NULL) {
      void *data = (char *)LockResource(header);

      if (data != NULL) {
        HANDLE file = CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                                 NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file != INVALID_HANDLE_VALUE) {
          DWORD size = SizeofResource(0, src);
          DWORD bytes;

          WriteFile(file, data, size, &bytes, nullptr);
          CloseHandle(file);

          ret = true;
        }
      }
      UnlockResource(header);

      FreeResource(header);
    }
  }

  return ret;
}

bool ClientFolder::create_directory(std::wstring directory) {
  bool result = CreateDirectory(directory.c_str(), NULL);

  if (result) {
    clog::info << "Created " << ST::string(directory) << clog::endl;
    return true;
  } else {
    DWORD last_error = GetLastError();

    if (last_error == ERROR_ALREADY_EXISTS)
      return true;
    else {
      clog::error << "Unable to create " << ST::string(directory)
                  << ", GetLastError() was " << last_error << clog::endl;
      return false;
    }
  }
}

// document as "relative to config.json"
std::wstring ClientFolder::resolve_path(std::wstring file) {
  std::wstring joined = directory + L"\\" + file;

  // default_config["window"]["meta"]["icon"] = ST::string(directory +
  // p_krunker);

  FILE *f = _wfopen(joined.c_str(), L"r");

  if (f) {
    fclose(f);
    return joined;
  }

  return file;
}

std::wstring ClientFolder::relative_path(std::wstring path) {
  return ST::replace_all(path, directory + L"\\", L"");
}

bool ClientFolder::create(std::wstring name) {
  bool ret = true;

  PWSTR ppsz_path;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppsz_path);

  if (!SUCCEEDED(hr))
    return false;

  directory = ppsz_path;

  CoTaskMemFree(ppsz_path);

  directory += L"\\" + name;

  if (create_directory(directory)) {
    if (writeResource(directory + p_chief, ICON_CHIEF))
      clog::info << "Created " << ST::string(directory + p_chief) << clog::endl;
    if (writeResource(directory + p_krunker, ICON_KRUNKER))
      clog::info << "Created " << ST::string(directory + p_krunker)
                 << clog::endl;

    for (std::wstring sdir : directories) {
      if (!create_directory(directory + sdir))
        ret = false;
    }

    clog::logs = directory + p_logs;
  } else {
    clog::error << "Unable to create root folder" << clog::endl;
    ret = false;
  }

  std::string config_buffer;

  if (loadResource(JSON_CONFIG, config_buffer)) {
    rapidjson::ParseResult ok =
        default_config.Parse(config_buffer.data(), config_buffer.size());

    if (!ok)
      clog::error << "Error parsing default config: "
                  << GetParseError_En(ok.Code()) << " (" << ok.Offset() << ")"
                  << clog::endl;
  } else
    clog::error << "Unable to load default config" << clog::endl;

  return ret;
}

bool ClientFolder::load_config() {
  rapidjson::Document new_config;

  {
    std::string config_buffer;

    if (IOUtil::readFile(directory + p_config, config_buffer)) {
      rapidjson::ParseResult ok =
          new_config.Parse(config_buffer.data(), config_buffer.size());

      if (!ok) {
        clog::error << "Error parsing default config: "
                    << GetParseError_En(ok.Code()) << " (" << ok.Offset() << ")"
                    << clog::endl;
        new_config.CopyFrom(default_config, new_config.GetAllocator());
      }
    } else {
      new_config.CopyFrom(default_config, new_config.GetAllocator());
    }
  }

  config.CopyFrom(
      TraverseCopy(new_config, default_config, config.GetAllocator()),
      config.GetAllocator());

  if (new_config.IsObject() && new_config.HasMember("userscripts") &&
      new_config["userscripts"].IsObject())
    config["userscripts"] =
        rapidjson::Value(new_config["userscripts"], config.GetAllocator());

  clog::debug << "Config loaded" << clog::endl;

  save_config();

  return true;
}

bool ClientFolder::save_config() {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  config.Accept(writer);

  if (IOUtil::writeFile(directory + p_config,
                        std::string(buffer.GetString(), buffer.GetSize()))) {
    clog::debug << "Config saved" << clog::endl;
    return true;
  } else
    return false;
}