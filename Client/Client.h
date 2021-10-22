#pragma once

class Client {
private:
	Updater updater;
	WebView2Installer installer;
	ClientFolder folder;
	KrunkerWindow game;
	KrunkerWindow social;
	KrunkerWindow editor;
	KrunkerWindow documents;
	DiscordEventHandlers presence_events = nullptr;
	HINSTANCE inst;
	int cmdshow;
	void rpc_loading();
	void install_runtimes();
	bool game_message(JSMessage msg);
	bool navigation_cancelled(ICoreWebView2* sender, Uri uri);
	void listen_navigation(WebView2Window& window);
public:
	Client(HINSTANCE h, int c);
	bool create();
	int messages();
};