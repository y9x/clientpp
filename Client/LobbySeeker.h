#pragma once
#include <functional>
#include <rapidjson/document.h>

struct Game {
	int region_id(std::string region);
	int mode, region;
	size_t players, max_players;
	std::string id, map;
	std::string link();
	bool custom;
	bool full();
	bool operator < (Game c);
	Game(const rapidjson::Value& data);
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