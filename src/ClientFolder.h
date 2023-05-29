#pragma once
#include <rapidjson/document.h>
#include <string>
#include <vector>

class ClientFolder {
public:
  std::wstring directory;
  std::wstring p_profile = L"\\Profile";
  std::wstring p_scripts = L"\\Scripts";
  std::wstring p_styles = L"\\Styles";
  std::wstring p_swapper = L"\\Swapper";
  std::wstring p_config = L"\\config.json";
  std::wstring p_chief = L"\\Chief.ico";
  std::wstring p_krunker = L"\\Krunker.ico";
  std::wstring p_logs = L"\\Logs";
  rapidjson::Document defaultConfig;
  rapidjson::Document config;
  bool create(std::wstring name);
  bool createDirectory(std::wstring directory);
  bool loadConfig();
  bool saveConfig();
  std::wstring relativePath(std::wstring path);
  std::wstring resolvePath(std::wstring file);

private:
  std::vector<std::wstring> directories{p_logs, p_scripts, p_styles, p_swapper,
                                        p_profile};
};