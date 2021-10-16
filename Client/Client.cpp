#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <atlbase.h>
#include <atlwin.h>
#include <atlenc.h>
#include <stdlib.h>
#include <tchar.h>
#include <thread>
#include "../Utils/StringUtil.h"
#include "../Utils/Uri.h"
#include "./Log.h"
#include "./Updater.h"
#include "./WebView2Installer.h"
#include "./KrunkerWindow.h"
#include <discord_rpc.h>
#include <discord_register.h>
#include "./Client.h"

using namespace StringUtil;
using Microsoft::WRL::Callback;

bool Client::navigation_cancelled(ICoreWebView2* sender, Uri uri) {
	bool kru_owns = uri.host_owns(L"krunker.io");
	bool cancel = false;
	std::wstring uhost = uri.host();
	WebView2Window* send = 0;

	if (kru_owns && uri.pathname() == krunker_game) send = &game;
	else if (kru_owns && uri.pathname() == krunker_social) send = &social;
	else if (kru_owns && uri.pathname() == krunker_editor) send = &editor;
	else {
		cancel = true;
		ShellExecute(NULL, L"open", uri.href.c_str(), L"", L"", SW_SHOW);
	}

	// if send->webview exists
	if (send && send->webview != sender) {
		cancel = true;
		send->get(inst, cmdshow, [this, uri, send](bool newly_created) {
			if (newly_created) listen_navigation(*send);
			send->webview->Navigate(uri.href.c_str());
		});
	}

	return cancel;
}

void Client::listen_navigation(WebView2Window& window) {
	EventRegistrationToken token;

	window.webview->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
		LPWSTR urip;
		args->get_Uri(&urip);
		if (navigation_cancelled(sender, urip)) args->put_Handled(true);
		else args->put_NewWindow(sender);

		return S_OK;
	}).Get(), &token);

	window.webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
		LPWSTR urip;
		args->get_Uri(&urip);
		if (navigation_cancelled(sender, urip)) args->put_Cancel(true);

		return S_OK;
	}).Get(), &token);
}

void Client::game_message(JSMessage msg) {
	if (msg.event == "rpc_uninit") {
		Discord_ClearPresence();
		// Discord_Shutdown();
	}else if (msg.event == "rpc") {
		if (folder.config["rpc"]["enabled"]) {
			DiscordRichPresence presence;
			memset(&presence, 0, sizeof(presence));
			
			int64_t start = msg.args[0];

			std::string
				user = msg.args[1],
				map = msg.args[2],
				mode = msg.args[3];
			
			presence.startTimestamp = start;
			presence.largeImageKey = "icon";

			presence.state = mode.c_str();
			presence.details = map.c_str();

			if (folder.config["rpc"]["name"]) presence.largeImageText = user.c_str();
			else presence.state = "In game";

			clog::info << "Set presence" << clog::endl;

			Discord_UpdatePresence(&presence);
		}
	}
}

Client::Client(HINSTANCE h, int c)
	: inst(h)
	, cmdshow(c)
	, updater(client_version, "https://y9x.github.io", "/userscripts/serve.json")
	, installer("https://go.microsoft.com", "/fwlink/p/?LinkId=2124703")
	, folder(L"GC++")
	, game(folder, { 0.8, 0.8 }, client_title, krunker_game, [this]() { listen_navigation(game); }, [this](JSMessage msg) { game_message(msg); })
	, social(folder, { 0.4, 0.6 }, (std::wstring(client_title) + L": Social").c_str(), krunker_social, [this]() { listen_navigation(social); })
	, editor(folder, { 0.4, 0.6 }, (std::wstring(client_title) + L": Editor").c_str(), krunker_editor, [this]() { listen_navigation(editor); })
{
	memset(&presence_events, 0, sizeof(presence_events));
	Discord_Initialize(client_discord_rpc, &presence_events, 1, NULL);
	
	if (!folder.create()) {
		clog::debug << "Error creating folder" << clog::endl;
		MessageBox(NULL, L"Error creating folder. See Error.log.", client_title, MB_OK);
		return;
	}
	
	folder.load_config();

	if (folder.config["rpc"]["enabled"]) {
		DiscordRichPresence presence;
		memset(&presence, 0, sizeof(presence));

		presence.startTimestamp = time(0);
		presence.largeImageKey = "icon";

		presence.state = "Loading";

		Discord_UpdatePresence(&presence);
	}

	HRESULT coinit = CoInitialize(NULL);
	if (!SUCCEEDED(coinit)) MessageBox(NULL, (L"COM could not be initialized. CoInitialize returned " + Convert::wstring(std::to_string(coinit))).c_str(), client_title, MB_OK);

	clog::info << "Main initialized" << clog::endl;

	if (!installer.Installed()) {
		clog::warn << "WebView2 Runtime not installed, prompting installation" << clog::endl;
		if (MessageBox(NULL, L"You are missing runtimes. Install the WebView2 Runtime?", client_title, MB_YESNO) == IDYES) {
			WebView2Installer::Error error;
			MessageBox(NULL, L"Relaunch the client after installation is complete.", client_title, MB_OK);
			if (!installer.Install(error)) switch (error) {
			case WebView2Installer::Error::CantOpenProcess:
				clog::error << "CantOpenProcess during WebView2 installation" << clog::endl;
				MessageBox(NULL, (L"Unable to open " + installer.bin + L". You will need to run the exe manually.").c_str(), client_title, MB_OK);
				break;
			case WebView2Installer::Error::NoBytesDownloaded:
				clog::error << "NoBytesDownloaded during WebView2 installation" << clog::endl;
				MessageBox(NULL, L"Unable to download MicrosoftEdgeWebview2Setup. Relaunch the client then try again.", client_title, MB_OK);
				break;
			default:
				clog::error << "Unknown error " << (int)error << " during WebView2 installation" << clog::endl;
				MessageBox(NULL, L"An unknown error occurred.", client_title, MB_OK);
				break;
			}
		}
		else MessageBox(NULL, L"Cannot continue without runtimes. Client will now exit.", client_title, MB_OK);
		return;
	}

	game.create(inst, cmdshow);

	// checking updates causes delay
	new std::thread([this]() {
		std::string update_url;
		if (updater.UpdatesAvailable(update_url) && MessageBox(game.m_hWnd, L"A new client update is available. Download?", client_title, MB_YESNO) == IDYES) {
			ShellExecute(game.m_hWnd, L"open", Convert::wstring(update_url).c_str(), L"", L"", SW_SHOW);
			game.open = false;
			return;
		}
	});
}
int Client::messages() {
	MSG msg;
	BOOL ret;
	while (game.open && (ret = GetMessage(&msg, 0, 0, 0))) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		game.on_dispatch();
		social.on_dispatch();
		editor.on_dispatch();
	}

	return EXIT_SUCCESS;
}

int APIENTRY WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, _In_ LPSTR cmdline, _In_ int nCmdShow) {
	Client client(hInstance, nCmdShow);
	return client.messages();
}