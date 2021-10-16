#pragma once

constexpr const long double client_version = 0.05;
constexpr const char* client_discord_rpc = "898655439300993045";
constexpr const wchar_t* client_title = L"Guru Client++";
constexpr const wchar_t* krunker_game = L"/";
constexpr const wchar_t* krunker_editor = L"/editor.html";
constexpr const wchar_t* krunker_social = L"/social.html";

class Client {
private:
	Updater updater;
	WebView2Installer installer;
	ClientFolder folder;
	KrunkerWindow game;
	KrunkerWindow social;
	KrunkerWindow editor;
	DiscordEventHandlers presence_events;
	HINSTANCE inst;
	int cmdshow;
	bool navigation_cancelled(ICoreWebView2* sender, Uri uri);
	void listen_navigation(WebView2Window& window);
	void game_message(JSMessage msg);
public:
	Client(HINSTANCE h, int c);
	int messages();
};