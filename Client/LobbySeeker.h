#pragma once
#include <functional>


class LobbySeeker {
public:
	static std::vector<std::string> modes;
	// { short, long }
	static std::vector<std::pair<std::string, std::string>> regions;
public:
	LobbySeeker() {}
	std::string seek(int region, int mode, std::string map);
};