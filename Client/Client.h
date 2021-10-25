#pragma once
#include "./Updater.h"
#include "./ClientFolder.h"
#include "./KrunkerWindow.h"
#include "./WebView2Installer.h"
#include "../Utils/Uri.h"
#include <discord_register.h>
#include <discord_rpc.h>

class Client {
private:
	Updater updater;
	WebView2Installer installer;
	ClientFolder folder;
	KrunkerWindow game;
	KrunkerWindow social;
	KrunkerWindow editor;
	KrunkerWindow documents;
	DiscordEventHandlers presence_events{};
	HINSTANCE inst;
	int cmdshow;
	void rpc_loading();
	void install_runtimes();
	bool game_message(JSMessage msg);
	bool navigation_cancelled(ICoreWebView2* sender, Uri uri);
	void listen_navigation(KrunkerWindow& window);
public:
	Client(HINSTANCE h, int c);
	bool create();
	int messages();
};