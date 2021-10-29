#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.hpp>
#include <json.hpp>
#include "./LobbySeeker.h"
#include "../Utils/StringUtil.h"
#include "./Log.h"

using namespace StringUtil;
using JSON = nlohmann::json;

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

Game::Game(JSON data)
	: id(data[0])
	, region(region_id(data[1]))
	, map(data[4]["i"])
	, mode(data[4]["g"])
	, players(data[2])
	, max_players(data[3])
{}

bool Game::operator < (Game c) {
	return players < c.players;
}

bool Game::full() {
	return players == max_players;
}

std::string Game::link() {
	return "https://krunker.io/?game=" + id;
}

std::string LobbySeeker::seek() {
	if (auto res = api.Get("/game-list?hostname=krunker.io"))try {
		JSON data = JSON::parse(res->body);
		
		std::vector<Game> games;

		for (JSON data : data["games"]) games.push_back(data);

		clog::info << "sorting.." << clog::endl;
		std::sort(games.begin(), games.end());
		clog::info << "sorted" << clog::endl;

		for (Game game : games) {
			if (game.full()) continue;
			if (region != -1 && game.region != region) continue;
			if (mode != -1 && game.mode != mode) continue;
			if (use_map && Manipulate::lowercase(game.map) != Manipulate::lowercase(map)) continue;

			return game.link();
		}
	}
	catch (JSON::parse_error err) {
		clog::error << "Unable to parse game list: " << err.what() << clog::endl;
	}
	catch (JSON::type_error err) {
		clog::error << "Unable to process game list: " << err.what() << clog::endl;
	}


	clog::error << "Error finding game" << clog::endl;
	return "https://krunker.io";
}