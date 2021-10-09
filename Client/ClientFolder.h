#pragma once
#include <string>
#include "../Utils/JSON.h"

class ClientFolder {
public:
	std::wstring directory;
	std::wstring p_profile = L"\\Profile";
	std::wstring p_scripts = L"\\Scripts";
	std::wstring p_styles = L"\\Styles";
	std::wstring p_swapper = L"\\Swapper";
	std::wstring p_config = L"\\Config.json";
	std::wstring p_guru = L"\\Guru.ico";
	std::wstring p_clientpp = L"\\Client++.ico";
	std::wstring p_log = L"\\Log.txt";
	JSON default_config = JSON::object();
	JSON config = JSON::object();
	ClientFolder(std::wstring name);
	bool load_config();
	bool save_config();
private:
	std::wstring name;
	std::vector<std::wstring> directories{ p_scripts, p_styles, p_swapper, p_profile };
	int last_error = 0;
	bool error_creating = false;
};