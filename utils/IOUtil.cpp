#define _CRT_SECURE_NO_WARNINGS
#include "./IOUtil.h"
#include "./StringUtil.h"
#include <fstream>
#include <sstream>
#include <windows.h>

namespace IOUtil {
bool DirectoryIterator::test_filename() {
  std::string f = file();
  return f == "." || f == "..";
}
DirectoryIterator::DirectoryIterator(std::string f, std::string filter)
    : folder(f) {
  std::string wsfind = folder;
  if (wsfind.back() != '\\')
    wsfind += '\\';
  wsfind += filter;
  folder_ws = ST::wstring(wsfind);
}

DirectoryIterator::~DirectoryIterator() { stop(); }

void DirectoryIterator::stop() {
  if (stopped)
    return;

  stopped = true;
  FindClose(find);
}

std::string DirectoryIterator::file() {
  return ST::string(find_data.cFileName);
}
std::string DirectoryIterator::path() { return folder + "\\" + file(); }

bool DirectoryIterator::operator++() {
  cnext = false;

  if (first) {
    first = false;
    if ((find = FindFirstFile(folder_ws.c_str(), &find_data)) ==
        INVALID_HANDLE_VALUE)
      return stopped = true, false;
  } else if (!FindNextFile(find, &find_data))
    return false;

  if (test_filename())
    return operator++();

  return cnext = true;
}

bool WDirectoryIterator::test_filename() {
  std::wstring f = file();
  return f == L"." || f == L"..";
}

WDirectoryIterator::WDirectoryIterator(std::wstring f, std::wstring filter)
    : folder(f), filter(filter) {
  if (folder.back() == '\\')
    folder.pop_back();
}

WDirectoryIterator::~WDirectoryIterator() { stop(); }

void WDirectoryIterator::stop() {
  if (stopped)
    return;

  stopped = true;
  FindClose(find);
}

std::wstring WDirectoryIterator::file() const { return find_data.cFileName; }

std::wstring WDirectoryIterator::path() const {
  return folder + L"\\" + file();
}

bool WDirectoryIterator::operator++() {
  cnext = false;

  if (first) {
    first = false;
    if ((find = FindFirstFile((folder + L"\\" + filter).c_str(), &find_data)) ==
        INVALID_HANDLE_VALUE)
      return stopped = true, false;
  } else if (!FindNextFile(find, &find_data))
    return false;

  if (test_filename())
    return operator++();

  return cnext = true;
}

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