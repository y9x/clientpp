#pragma once
#include <string>
#include <json.hpp>

class ClientFolder {
public:
	std::wstring directory;
	std::wstring p_profile = L"\\Profile";
	std::wstring p_scripts = L"\\Scripts";
	std::wstring p_styles = L"\\Styles";
	std::wstring p_swapper = L"\\Swapper";
	std::wstring p_config = L"\\Config.json";
	std::wstring p_chief = L"\\Chief.ico";
	std::wstring p_krunker = L"\\Krunker.ico";
	std::wstring p_logs = L"\\Logs";
	nlohmann::json default_config = nlohmann::json::object();
	nlohmann::json config = nlohmann::json::object();
	ClientFolder(std::wstring name);
	bool create();
	bool create_directory(std::wstring directory);
	bool load_config();
	bool save_config();
	std::wstring relative_path(std::wstring path);
	std::wstring resolve_path(std::wstring file);
private:
	std::wstring name;
	std::vector<std::wstring> directories{ p_logs, p_scripts, p_styles, p_swapper, p_profile };
};