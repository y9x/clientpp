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

constexpr const long double client_version = 0.02;
constexpr const wchar_t* client_title = L"Guru Client++";
constexpr const wchar_t* krunker_game = L"/";
constexpr const wchar_t* krunker_editor = L"/editor.html";
constexpr const wchar_t* krunker_social = L"/social.html";

using namespace StringUtil;
using Microsoft::WRL::Callback;

class Main {
private:
	Updater updater;
	WebView2Installer installer;
	ClientFolder folder;
	KrunkerWindow game;
	KrunkerWindow social;
	KrunkerWindow editor;
	HINSTANCE inst;
	int cmdshow;
	bool navigation_cancelled(ICoreWebView2* sender, Uri uri) {
		bool cancel = false;
		std::wstring uhost = uri.host();
		WebView2Window* send = 0;

		if (uri.host_owns(L"krunker.io")) {
			if (uri.pathname() == krunker_game) send = &game;
			else if (uri.pathname() == krunker_social) send = &social;
			else if (uri.pathname() == krunker_editor) send = &editor;
		}
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
	void listen_navigation(WebView2Window& window) {
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
public:
	Main(HINSTANCE h, int c)
		: inst(h)
		, cmdshow(c)
		, updater(client_version, "https://y9x.github.io", "/userscripts/serve.json")
		, installer("https://go.microsoft.com", "/fwlink/p/?LinkId=2124703")
		, folder(L"GC++")
		, game(folder, { 0.8, 0.8 }, L"Guru Client++", krunker_game)
		, social(folder, { 0.4, 0.6 }, L"Guru Client++", krunker_social)
		, editor(folder, { 0.4, 0.6 }, L"Social", krunker_editor)
	{
		CoInitialize(NULL);

		LOG_INFO("Main initialized");

		if (!installer.Installed()) {
			if (MessageBox(NULL, L"You are missing runtimes. Do you wish to install WebView2 Runtime?", client_title, MB_YESNO) == IDYES) {
				WebView2Installer::Error error;
				if (installer.Install(error))
					MessageBox(NULL, L"Relaunch the client after installation is complete.", client_title, MB_OK);
				else switch (error) {
				case WebView2Installer::Error::CantOpenProcess:
					MessageBox(NULL, (L"Couldn't open " + installer.bin + L". You will need to run the exe manually.").c_str(), client_title, MB_OK);
					break;
				default:
					MessageBox(NULL, L"An unknown error occurred.", client_title, MB_OK);
					break;
				}
			}
			else MessageBox(NULL, L"Cannot continue without runtimes, quitting...", client_title, MB_OK);
			return;
		}

		game.create(inst, cmdshow, [this]() {
			listen_navigation(game);
		});

		// checking updates causes delay
		new std::thread([this]() {
			std::string update_url;
			if (updater.UpdatesAvailable(update_url) && MessageBox(game.m_hWnd, L"A new client update is available. Download?", client_title, MB_YESNO) == IDYES) {
				ShellExecute(game.m_hWnd, L"open", Convert::wstring(update_url).c_str(), L"", L"", SW_SHOW);
				return;
			}
		});

		MSG msg;
		BOOL ret;

		while (game.open && (ret = GetMessage(&msg, 0, 0, 0))) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			game.on_dispatch();
			social.on_dispatch();
			editor.on_dispatch();
		}
	}
};

int APIENTRY WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, _In_ LPSTR cmdline, _In_ int nCmdShow) {
	Main m(hInstance, nCmdShow);
	return EXIT_SUCCESS;
}