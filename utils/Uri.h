#pragma once
#include <string>

// WebView2 Uri Interface
class Uri {
private:
public:
  Uri(std::wstring u);
  Uri(const wchar_t *u);
  std::wstring protocol();
  std::wstring host(bool remove_www = false);
  std::wstring origin();
  std::wstring path();
  std::wstring search();
  std::wstring pathname();
  std::wstring href;
  bool host_owns(std::wstring match);
};

// using SearchParams = std::map<std::wstring, std::wstring>