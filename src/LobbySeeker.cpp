#include "./LobbySeeker.h"
#include "../utils/StringUtil.h"
#include "./Log.h"
#include "./fetch.h"
#include <algorithm>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

// config.js
std::vector<std::string> seekModes = {"Free for All",
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
                                      "Classic FFA",
                                      "Deposit",
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
                                      "Kranked FFA",
                                      "Team Defender",
                                      "Deposit FFA",
                                      "Carrier",
                                      "All or Nothing",
                                      "Mag Dump",
                                      "Chaos Snipers",
                                      "Bighead FFA",
                                      "Zombies",
                                      "Impulsed"};

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

std::string seekLobby(const std::string &region, size_t mode, bool customs,
                      const std::string &map) {
  if (!map.size() && mode == -1)
    return "https://krunker.io/";

  std::string data =
      fetchGet("https://matchmaker.krunker.io/game-list?hostname=krunker.io");

  rapidjson::Document gameDoc;
  rapidjson::ParseResult ok = gameDoc.Parse(data.data(), data.size());

  if (!ok)
    clog::error << "Error parsing games: " << GetParseError_En(ok.Code())
                << " (" << ok.Offset() << ")" << clog::endl;

  std::vector<Game> games;

  for (const rapidjson::Value &data : gameDoc["games"].GetArray()) {
    Game game(data);

    if (game.isFull())
      continue;
    if (mode != -1 && game.config.mode != mode)
      continue;
    if (!customs && game.config.custom)
      continue;
    if (region != "" && game.region != region)
      continue;
    if (map.size() && ST::lowercase(game.config.map) != map)
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

  clog::error << "Error finding game" << clog::endl;
  return "https://krunker.io";
}