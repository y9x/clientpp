#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.hpp>
#include <rapidjson/document.h>
#include "./LobbySeeker.h"
#include "../Utils/StringUtil.h"
#include "./Log.h"

using namespace StringUtil;

httplib::Client api("https://matchmaker.krunker.io");

// config.js
std::vector<std::string> LobbySeeker::modes = { "Free for All", "Team Deathmatch", "Hardpoint", "Capture the Flag", "Parkour", "Hide & Seek", "Infected", "Race", "Last Man Standing", "Simon Says", "Gun Game", "Prop Hunt", "Boss Hunt", "unused", "unused", "Stalker", "King of the Hill", "One in the Chamber", "Trade", "Kill Confirmed", "Defuse", "Sharp Shooter", "Traitor", "Raid", "Blitz", "Domination", "Squad Deathmatch", "Kranked FFA" };

std::vector<std::pair<std::string, std::string>> LobbySeeker::regions = {
	{ "us-nj", "New York" },
	{ "us-il", "Chicago" },
	{ "us-tx", "Dallas" },
	{ "us-wa", "Seattle" },
	{ "us-ca-la", "Los Angeles" },
	{ "us-ga", "Atlanta" },
	{ "nl-ams", "Amsterdam" },
	{ "gb-lon", "London" },
	{ "de-fra", "Frankfurt" },
	{ "us-ca-sv", "Silicon Valley" },
	{ "au-syd", "Sydney" },
	{ "fr-par", "Paris" },
	{ "jb-hnd", "Tokyo" },
	{ "us-fl", "Miami" },
	{ "sgp", "Singapore" },
	{ "blr", "India" },
	{ "brz", "Brazil" },
	{ "me-bhn", "Middle East" },
	{ "af-ct", "South Africa" },
	{ "as-seoul", "South Korea" },
};

int Game::region_id(std::string region) {
	for (size_t mir = 0; mir < LobbySeeker::regions.size(); mir++) if (LobbySeeker::regions[mir].first == region)
		return (int)mir;

	return -1;
}

Game::Game(const rapidjson::Value& data)
	: id(data[0].GetString(), data[0].GetStringLength())
	, region(region_id(std::string(data[1].GetString(), data[1].GetStringLength())))
	, map(std::string(data[4]["i"].GetString(), data[4]["i"].GetStringLength()))
	, mode(data[4]["g"].GetInt())
	, players(data[2].GetInt())
	, max_players(data[3].GetInt())
	, custom(data[4]["c"].GetInt())
{}

bool Game::operator < (Game c) {
	return players > c.players; // && (max_players - 1 < players);
}

bool Game::full() {
	return players == max_players;
}

std::string Game::link() {
	return "https://krunker.io/?game=" + id;
}

std::string LobbySeeker::seek() {
	if (auto res = api.Get("/game-list?hostname=krunker.io")) {
		rapidjson::Document data;
		data.Parse(res->body.c_str(), res->body.length());
		
		std::vector<Game> games;

		for (const rapidjson::Value& data : data["games"].GetArray()) {
			Game game = data;

			if (game.full()) continue;
			if (region != -1 && game.region != region) continue;
			if (mode != -1 && game.mode != mode) continue;
			if (!customs && game.custom) continue;
			if (use_map && Manipulate::lowercase(game.map) != map) continue;

			games.push_back(game);
		}
		
		if (games.size()) {
			std::sort(games.begin(), games.end());
			return games[0].link();
		}
	}

	clog::error << "Error finding game" << clog::endl;
	return "https://krunker.io";
}