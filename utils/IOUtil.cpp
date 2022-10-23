#define _CRT_SECURE_NO_WARNINGS
#include "./IOUtil.h"
#include "./StringUtil.h"
#include <fstream>
#include <sstream>
#include <windows.h>

namespace IOUtil {
bool file_exists(const std::filesystem::path &path) {
  std::fstream t(path);
  if (!t)
    return false;
  return true;
}

bool read_file(const std::filesystem::path &path, std::string &out) {
  std::ifstream t(path);
  if (!t)
    return false;
  std::stringstream buffer;
  buffer << t.rdbuf();
  out = buffer.str();
  return true;
}

// for small data
bool write_file(const std::filesystem::path &path, const std::string &buffer) {
  std::ofstream t(path);
  if (!t)
    return false;
  t << buffer;
  t.close();
  return true;
}
}; // namespace IOUtil