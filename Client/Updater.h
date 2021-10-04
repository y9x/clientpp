#pragma once
#include "../Utils/JSON.h"

class Updater {
public:
	std::string host = "https://y9x.github.io";
	std::string path = "/userscripts/serve.json";
	long double version;
	JSON GetServing();
	bool UpdatesAvailable(std::string& url);
	Updater(long double version);
};