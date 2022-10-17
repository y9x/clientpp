#include "./EncodeURI.h"
#include <regex>
#include <sstream>

std::string encodeURIComponent(const std::string &decoded) {
  std::ostringstream oss;
  std::regex r("[!'\\(\\)*-.0-9A-Za-z_~]");

  for (const char &c : decoded) {
    std::string cw;
    cw += c;
    if (std::regex_match(cw, r))
      oss << c;
    else
      oss << "%" << std::uppercase << std::hex << (0xff & c);
  }

  return oss.str();
}

std::wstring encodeURIComponent(const std::wstring &decoded) {
  std::wostringstream oss;
  std::wregex r(L"[!'\\(\\)*-.0-9A-Za-z_~]");

  for (const wchar_t &c : decoded) {
    std::wstring cw;
    cw += c;
    if (std::regex_match(cw, r))
      oss << c;
    else
      oss << L"%" << std::uppercase << std::hex << (0xff & c);
  }

  return oss.str();
}
