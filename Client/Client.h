#pragma once


constexpr const long double client_version = 0.05;
constexpr const wchar_t* client_title = L"Guru Client++";
constexpr const wchar_t* krunker_game = L"/";
constexpr const wchar_t* krunker_editor = L"/editor.html";
constexpr const wchar_t* krunker_social = L"/social.html";

class GameInfo {
public:
	std::string thumbnail, region, build, map, id;
	bool custom, locked;
	int mode, kr, players, max_players;
	GameInfo(JSON data);
};

class Client {
private:
	Updater updater;
	WebView2Installer installer;
	ClientFolder folder;
	KrunkerWindow game;
	KrunkerWindow social;
	KrunkerWindow editor;
	HINSTANCE inst;
	DiscordEventHandlers presence_events;
	int cmdshow;
	bool navigation_cancelled(ICoreWebView2* sender, Uri uri);
	void listen_navigation(WebView2Window& window);
	void game_message(JSMessage msg);
public:
	Client(HINSTANCE h, int c);
	int messages();
};