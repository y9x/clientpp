#pragma once
#include "../Utils/JSON.h"

class Updater {
public:
	std::string host, path;
	Updater(long double version, std::string host, std::string path);
	long double version;
	JSON GetServing();
	bool UpdatesAvailable(std::string& url);
};