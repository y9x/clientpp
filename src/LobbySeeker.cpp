#include "./LobbySeeker.h"
#include "../utils/StringUtil.h"
#include "./Log.h"
#include <net.h>
#include <rapidjson/document.h>

// config.js
std::vector<std::string> LobbySeeker::modes = {"Free for All",
                                               "Team Deathmatch",
                                               "Hardpoint",
                                               "Capture the Flag",
                                               "Parkour",
                                               "Hide & Seek",
                                               "Infected",
                                               "Race",
                                               "Last Man Standing",
                                               "Simon Says",
                                               "Gun Game",
                                               "Prop Hunt",
                                               "Boss Hunt",
                                               "unused",
                                               "unused",
                                               "Stalker",
                                               "King of the Hill",
                                               "One in the Chamber",
                                               "Trade",
                                               "Kill Confirmed",
                                               "Defuse",
                                               "Sharp Shooter",
                                               "Traitor",
                                               "Raid",
                                               "Blitz",
                                               "Domination",
                                               "Squad Deathmatch",
                                               "Kranked FFA"};

std::vector<std::pair<std::string, std::string>> LobbySeeker::regions = {
    {"us-nj", "New York"},       {"us-il", "Chicago"},
    {"us-tx", "Dallas"},         {"us-wa", "Seattle"},
    {"us-ca-la", "Los Angeles"}, {"us-ga", "Atlanta"},
    {"nl-ams", "Amsterdam"},     {"gb-lon", "London"},
    {"de-fra", "Frankfurt"},     {"us-ca-sv", "Silicon Valley"},
    {"au-syd", "Sydney"},        {"fr-par", "Paris"},
    {"jb-hnd", "Tokyo"},         {"us-fl", "Miami"},
    {"sgp", "Singapore"},        {"blr", "India"},
    {"brz", "Brazil"},           {"me-bhn", "Middle East"},
    {"af-ct", "South Africa"},   {"as-seoul", "South Korea"},
};

int regionID(std::string region) {
  for (size_t mir = 0; mir < LobbySeeker::regions.size(); mir++)
    if (LobbySeeker::regions[mir].first == region)
      return (int)mir;

  return -1;
}

GameConfig::GameConfig(const rapidjson::Value &data)
    : map(data["i"].GetString()), mode(data["g"].GetInt()),
      custom(data["c"].GetInt()) {}

Game::Game(const rapidjson::Value &data)
    : id(data[0].GetString()), region(data[1].GetString()),
      players(data[2].GetInt()), max_players(data[3].GetInt()), config(data[4]),
      time(data[5].GetInt()) {}

bool Game::isFull() { return players == max_players; }

std::string Game::getLink() {
  return "https://krunker.io/?game=" + std::string(id);
}

// increase weight of game players if it is within the target range (4, 6)
size_t strongPlayers(size_t x) { return (x >= 4 || x <= 6) ? 8 : x; }

std::string LobbySeeker::seek() {
  if (!use_map && mode == -1)
    return "https://krunker.io";

  try {
    auto res = net::fetch_request(net::url(
        L"https://matchmaker.krunker.io/game-list?hostname=krunker.io"));

    rapidjson::Document data;
    data.Parse(res.data(), res.size());

    std::vector<Game> games;

    for (const rapidjson::Value &data : data["games"].GetArray()) {
      Game game(data);

      if (game.isFull())
        continue;
      if (mode != -1 && game.config.mode != mode)
        continue;
      if (!customs && game.config.custom)
        continue;
      if (region != -1 && regionID(game.region) != region)
        continue;
      if (use_map && ST::lowercase(game.config.map) != map)
        continue;

      games.push_back(game);
    }

    if (games.size()) {
      // game with the most time left
      std::stable_sort(
          games.begin(), games.end(),
          [](const Game &a, const Game &b) -> bool { return a.time < b.time; });

      // game with the most players
      std::stable_sort(
          games.begin(), games.end(), [](const Game &a, const Game &b) -> bool {
            return strongPlayers(a.players) < strongPlayers(b.players);
          });

      return games[0].getLink();
    }
  } catch (net::error &err) {
    clog::error << "Failure requesting matchmaker: " << err.what()
                << clog::endl;
  }

  clog::error << "Error finding game" << clog::endl;
  return "https://krunker.io";
}