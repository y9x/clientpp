#pragma once
#include <string>

struct UpdaterServing {
  std::string url;
  std::string version;
};

/// @brief Fetches the serving and returns if a newer version is available
extern bool updatesAvailable(const std::string &version, const std::string &url,
                             UpdaterServing &serving);
