#define _CRT_SECURE_NO_WARNINGS
#include "./Client.h"
#include "./Log.h"
#include "../Utils/StringUtil.h"
#include <shellapi.h>

constexpr const long double client_version = 0.10;
constexpr const char* client_discord_rpc = "";
// 899137303182716968
constexpr const wchar_t* client_title = L"Chief Client++";

namespace krunker {
	constexpr const wchar_t* game = L"/";
	constexpr const wchar_t* games = L"/games";
	constexpr const wchar_t* editor = L"/editor.html";
	constexpr const wchar_t* social = L"/social.html";
	constexpr const wchar_t* docs = L"/docs/";
	constexpr const wchar_t* tos = L"/docs/tos.html";
};

using namespace StringUtil;
using Microsoft::WRL::Callback;

bool Client::navigation_cancelled(ICoreWebView2* sender, Uri uri) {
	if (uri.host() == L"chief") return false;
	
	bool kru_owns = uri.host_owns(L"krunker.io");
	bool cancel = false;
	std::wstring pathname = uri.pathname();

	WebView2Window* send = 0;

	if (kru_owns) {
		if (pathname == krunker::game || pathname.starts_with(krunker::games)) send = &game;
		else if (pathname == krunker::social) send = &social;
		else if (pathname == krunker::editor) send = &editor;
		else if (pathname.starts_with(krunker::docs)) send = &documents;
	}
	if (!send) {
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
		else {
			args->put_Handled(true);
			sender->Navigate(urip);
		}

		return S_OK;
	}).Get(), &token);

	window.webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
		LPWSTR urip;
		args->get_Uri(&urip);
		if (navigation_cancelled(sender, urip)) args->put_Cancel(true);

		return S_OK;
	}).Get(), &token);
}

void Client::rpc_loading() {
	DiscordRichPresence presence;
	memset(&presence, 0, sizeof(presence));

	presence.startTimestamp = time(0);
	presence.largeImageKey = "icon";

	presence.state = "Loading";

	Discord_UpdatePresence(&presence);
}

bool Client::game_message(JSMessage msg) {
	switch (msg.event) {
	case IM::rpc_init:

		if (!folder.config["rpc"]["enabled"]) break;

		rpc_loading();

		break;
	case IM::rpc_clear:

		Discord_ClearPresence();

		break;
	case IM::rpc_update: {

		if (!folder.config["rpc"]["enabled"]) break;

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

		// clog::info << "Set presence" << clog::endl;

		Discord_UpdatePresence(&presence);

	} break;
	default:
		return false;
	}

	return true;
}

void Client::install_runtimes() {
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
}

Client::Client(HINSTANCE h, int c)
	: inst(h)
	, cmdshow(c)
	, updater(client_version, "https://6ct.github.io", "/serve/updates.json")
	, installer("https://go.microsoft.com", "/fwlink/p/?LinkId=2124703")
	, folder(L"GC++") // test unicode support L"크롬 플래그 새끼"	
	, social(folder, { 0.4, 0.6 }, (std::wstring(client_title) + L": Social").c_str(), krunker::social, [this]() { listen_navigation(social); })
	, game(folder, { 0.8, 0.8 }, client_title, krunker::game, [this]() { listen_navigation(game); }, [this](JSMessage msg) -> bool { return game_message(msg); }, []() { PostQuitMessage(EXIT_SUCCESS); })
	, editor(folder, { 0.4, 0.6 }, (std::wstring(client_title) + L": Editor").c_str(), krunker::editor, [this]() { listen_navigation(editor); })
	, documents(folder, { 0.4, 0.6 }, (std::wstring(client_title) + L": Documents").c_str(), krunker::tos, [this]() { listen_navigation(documents); })
{}

bool Client::create() {
	memset(&presence_events, 0, sizeof(presence_events));
	Discord_Initialize(client_discord_rpc, &presence_events, 1, NULL);
	
	if (!folder.create()) {
		clog::debug << "Error creating folder" << clog::endl;
		MessageBox(NULL, L"Error creating folder. See Error.log.", client_title, MB_OK);
		return false;
	}
	
	folder.load_config();

	if (folder.config["rpc"]["enabled"]) rpc_loading();

	HRESULT coinit = CoInitialize(NULL);
	if (!SUCCEEDED(coinit)) MessageBox(NULL, (L"COM could not be initialized. CoInitialize returned " + Convert::wstring(std::to_string(coinit))).c_str(), client_title, MB_OK);

	clog::info << "Main initialized" << clog::endl;

	if (!installer.Installed()) {
		install_runtimes();
		return false;
	}

	game.can_fullscreen = true;
	documents.background = RGB(0xFF, 0xFF, 0xFF);
	
	switch (game.create(inst, cmdshow)) {
	case KrunkerWindow::Status::MissingRuntime:
		clog::error << "WebView2 installation check failed. Unable to create game window." << clog::endl;
		install_runtimes();
		return false;
		break;
	case KrunkerWindow::Status::UnknownError:
		std::wstringstream sstream;
		sstream << std::hex << game.last_herror;
		
		game.MessageBox(
			(std::wstring(L"An unknown error ocurred during game creation. Create a issue on GitHub ( https://github.com/6ct/clientpp/issues ) and provide the following error details:\n") +
			L"Error code: 0x" + sstream.str()).c_str(), client_title, MB_ICONERROR);

		return false;
		break;
	};

	// checking updates causes delay
	new std::thread([this]() {
		UpdaterServing serving;
		if (updater.UpdatesAvailable(serving) && MessageBox(game.m_hWnd, L"A new client update is available. Download?", client_title, MB_YESNO) == IDYES) {
			ShellExecute(game.m_hWnd, L"open", Convert::wstring(serving.url).c_str(), L"", L"", SW_SHOW);
			if(::IsWindow(game.m_hWnd))game.DestroyWindow();
			// PostThreadMessage(0, WM_QUIT, EXIT_SUCCESS);
		}
	});

	return true;
}

int Client::messages() {
	MSG msg;
	BOOL ret;
	
	while (ret = GetMessage(&msg, 0, 0, 0)) {
		game.on_dispatch();
		social.on_dispatch();
		editor.on_dispatch();
		documents.on_dispatch();

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return EXIT_SUCCESS;
}

int APIENTRY WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, _In_ LPSTR cmdline, _In_ int nCmdShow) {
	Client client(hInstance, nCmdShow);
	if (client.create()) return client.messages();
	else return false;
}