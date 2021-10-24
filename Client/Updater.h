#pragma once
#include <string>

struct UpdaterServing {
	std::string url;
	double version = 0;
};

class Updater {
public:
	std::string host, path;
	Updater(long double version, std::string host, std::string path);
	long double version;
	bool GetServing(UpdaterServing& serving);
	bool UpdatesAvailable(UpdaterServing& serving);
};