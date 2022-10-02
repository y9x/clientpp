#pragma once
#include <string>

struct UpdaterServing {
	std::string url;
	double version = 0;
};

class Updater {
public:
	std::wstring url;
	Updater(long double version, std::wstring url);
	long double version;
	bool GetServing(UpdaterServing& serving);
	bool UpdatesAvailable(UpdaterServing& serving);
};