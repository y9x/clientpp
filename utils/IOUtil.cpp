#include "./IOUtil.h"
#include <fstream>
#include <sstream>

bool IOUtil::fileExists(const std::filesystem::path &path) {
  std::fstream t(path);
  if (!t)
    return false;
  return true;
}

bool IOUtil::readFile(const std::filesystem::path &path, std::string &out) {
  std::ifstream t(path);
  if (!t)
    return false;
  std::stringstream buffer;
  buffer << t.rdbuf();
  out = buffer.str();
  return true;
}

bool IOUtil::writeFile(const std::filesystem::path &path,
                       const std::string &buffer) {
  std::ofstream t(path);
  if (!t)
    return false;
  t << buffer;
  t.close();
  return true;
}