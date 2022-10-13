#pragma once
#include <functional>
#include <rapidjson/document.h>

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
  size_t max_players;
  GameConfig config;
  int time;
  Game(const rapidjson::Value &data);
  bool isFull();
  std::string getLink();
};

class LobbySeeker {
public:
  static std::vector<std::string> modes;
  // { short, long }
  static std::vector<std::pair<std::string, std::string>> regions;
  size_t region = -1;
  size_t mode = -1;
  bool customs = false;
  bool use_map = false;
  std::string map;
  std::string seek();
};