#pragma once
#include <string>

// WebView2 Uri Interface
class Uri {
private:
	std::wstring uri;
public:
	Uri(std::wstring u);
	std::wstring protocol();
	std::wstring host(bool remove_www = false);
	std::wstring origin();
	std::wstring path();
	std::wstring search();
	std::wstring pathname();
	bool HostEquals(std::wstring match);
};

// using SearchParams = std::map<std::wstring, std::wstring>