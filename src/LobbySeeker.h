#pragma once
#include <map>
#include <rapidjson/document.h>
#include <string>
#include <vector>

struct GameConfig {
  bool custom;
  const char *map;
  int mode;
  GameConfig(const rapidjson::Value &data);
};

struct Game {
  const char *id;
  const char *region;
  size_t players;
  size_t maxPlayers;
  GameConfig config;
  int time;
  Game(const rapidjson::Value &data);
  bool isFull();
  std::string getLink();
};

// { short mode name, long mode name }
extern std::vector<std::string> seekModes;

/// @brief
/// @param region
/// @param mode If -1, no mode will be searched for
/// @param customs
/// @param map If empty, any map will be searched for.
/// @return URL to found game.
extern std::string seekLobby(const std::string &region, size_t mode,
                             bool customs, const std::string &map);
