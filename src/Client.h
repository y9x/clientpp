#pragma once
#include "./Updater.h"
#include "./ClientFolder.h"
#include "./KrunkerWindow.h"
#include "./WebView2Installer.h"
#include "../utils/Uri.h"
#include "./AccountManager.h"
#include <discord_register.h>
#include <discord_rpc.h>

class Client
{
private:
	static const long double version;
	static const char *discord_rpc;
	static const wchar_t *title;
	AccountManager accounts;
	Updater updater;
	WebView2Installer installer;
	ClientFolder folder;
	KrunkerWindow game;
	KrunkerWindow social;
	KrunkerWindow editor;
	KrunkerWindow scripting;
	KrunkerWindow documents;
	DiscordEventHandlers presence_events{};
	HINSTANCE inst;
	HMODULE shcore;
	int cmdshow;
	void rpc_loading();
	void install_runtimes();
	bool on_message(JSMessage msg, KrunkerWindow &window);
	bool navigation_cancelled(ICoreWebView2 *sender, Uri uri);
	void listen_navigation(KrunkerWindow &window);

public:
	Client(HINSTANCE h, int c);
	bool create();
	int messages();
};