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

constexpr const long double client_version = 0.03;
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
		, game(folder, { 0.8, 0.8 }, L"Guru Client++", krunker_game, [this]() { listen_navigation(game); })
		, social(folder, { 0.4, 0.6 }, L"Guru Client++: Social", krunker_social, [this]() { listen_navigation(social); })
		, editor(folder, { 0.4, 0.6 }, L"Guru Client++: Editor", krunker_editor, [this]() { listen_navigation(editor); })
	{
		if (!folder.create()) {
			clog::debug << "Error creating folder" << clog::endl;
			MessageBox(NULL, L"Error creating folder. See Error.log.", client_title, MB_OK);
			return;
		}

		folder.load_config();
		
		HRESULT coinit = CoInitialize(NULL);
		if (!SUCCEEDED(coinit)) MessageBox(NULL, (L"COM could not be initialized. CoInitialize returned " + Convert::wstring(std::to_string(coinit))).c_str(), client_title, MB_OK);

		clog::info << "Main initialized" << clog::endl;

		if (!installer.Installed()) {
			clog::warn << "WebView2 Runtime not installed, prompting installation" << clog::endl;
			if (MessageBox(NULL, L"You are missing runtimes. Do you wish to install WebView2 Runtime?", client_title, MB_YESNO) == IDYES) {
				WebView2Installer::Error error;
				MessageBox(NULL, L"Relaunch the client after installation is complete.", client_title, MB_OK);
				if (!installer.Install(error)) switch (error) {
				case WebView2Installer::Error::CantOpenProcess:
					clog::error << "CantOpenProcess during WebView2 installation" << clog::endl;
					MessageBox(NULL, (L"Couldn't open " + installer.bin + L". You will need to run the exe manually.").c_str(), client_title, MB_OK);
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
				return;
			}
		});
	}
	int messages() {
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
};

int APIENTRY WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, _In_ LPSTR cmdline, _In_ int nCmdShow) {
	Main main(hInstance, nCmdShow);
	return main.messages();
}