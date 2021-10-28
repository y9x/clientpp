#pragma once
#include <functional>
#include <json.hpp>

class Game {
private:
	int region_id(std::string region);
public:
	int mode, region;
	size_t players, max_players;
	std::string id, map;
	Game(nlohmann::json data);
	bool operator < (Game c);
	bool full();
	std::string link();
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