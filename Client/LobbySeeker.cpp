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

std::string LobbySeeker::seek(int region, int mode, std::string map) {
	map = Manipulate::lowercase(map);
	
	if (auto res = api.Get("/game-list?hostname=krunker.io"))try {
		JSON data = JSON::parse(res->body);

		for (JSON game : data["games"]) {
			JSON meta = game[4];
			int meta_mode = meta["g"];
			int meta_region = -1;
			std::string meta_sregion = game[1];

			for (int mir = 0; mir < regions.size(); mir++) if (regions[mir].first == meta_sregion) {
				meta_region = mir;
				break;
			}

			std::string meta_id = game[0];
			std::string meta_map = Manipulate::lowercase(meta["i"]);
				
			if (region != meta_region) { clog::warn << "game not specified region" << clog::endl; continue; }
			if (mode != -1 /*ANY*/ && meta_mode != mode) { clog::warn << "game not specified mode" << clog::endl; continue; }
			if (map != "" && meta_map != map) { clog::warn << "game not specified map" << clog::endl; continue; }

			clog::info << "Valid " << game << clog::endl;
				
			return "https://krunker.io/?game=" + meta_id;

			break;
		}

		clog::error << "Error finding game" << clog::endl;
		return "https://krunker.io";
	}
	catch (JSON::parse_error err) {
		clog::error << "Unable to parse game list: " << err.what() << clog::endl;
	}
	catch (JSON::type_error err) {
		clog::error << "Unable to process game list: " << err.what() << clog::endl;
	}
}