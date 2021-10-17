#pragma once
#include <string>
#include <json.hpp>

class Updater {
public:
	std::string host, path;
	Updater(long double version, std::string host, std::string path);
	long double version;
	nlohmann::json GetServing();
	bool UpdatesAvailable(std::string& url);
};