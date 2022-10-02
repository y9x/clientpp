#pragma once
#include <string>

struct UpdaterServing {
  std::string url;
  std::string version;
};

class Updater {
private:
  std::wstring url;
  std::string version;

public:
  Updater(std::string version, std::wstring url);
  bool GetServing(UpdaterServing &serving);
  bool UpdatesAvailable(UpdaterServing &serving);
};