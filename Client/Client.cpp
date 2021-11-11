#define _CRT_SECURE_NO_WARNINGS
#include "./Client.h"
#include "./Log.h"
#include "../Utils/StringUtil.h"
#include <shellapi.h>
#include <ShellScalingApi.h>

using namespace StringUtil;
using Microsoft::WRL::Callback;

namespace krunker {
	constexpr const wchar_t* game = L"/";
	constexpr const wchar_t* games = L"/games";
	constexpr const wchar_t* editor = L"/editor.html";
	constexpr const wchar_t* social = L"/social.html";
	constexpr const wchar_t* viewer = L"/viewer.html";
	constexpr const wchar_t* scripting = L"/scripting.html";
	constexpr const wchar_t* docs = L"/docs/";
	constexpr const wchar_t* tos = L"/docs/tos.html";
};

const long double Client::version = 0.17;
const char* Client::discord_rpc = /*899137303182716968*/ "";
const wchar_t* Client::title = L"Chief Client++";

bool Client::navigation_cancelled(ICoreWebView2* sender, Uri uri) {
	if (uri.host() == L"chief") return false;
	
	bool kru_owns = uri.host_owns(L"krunker.io");
	bool cancel = false;
	std::wstring pathname = uri.pathname();

	KrunkerWindow* send = nullptr;

	if (kru_owns) {
		if (uri.host_owns(L"docs.krunker.io") || pathname.starts_with(krunker::docs)) send = &documents;
		else if (pathname == krunker::game || pathname.starts_with(krunker::games)) send = &game;
		else if (pathname == krunker::social) send = &social;
		else if (pathname == krunker::editor) send = &editor;
		else if (pathname == krunker::scripting) send = &scripting;
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
			send->BringWindowToTop();
		});
	}

	return cancel;
}

void Client::listen_navigation(KrunkerWindow& window) {
	EventRegistrationToken token;

	window.webview->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
		LPWSTR urip;
		args->get_Uri(&urip);
		if (navigation_cancelled(sender, urip)) /* TODO: SET OPENER */ args->put_Handled(true);
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

bool Client::on_message(JSMessage msg, KrunkerWindow& window) {
	switch (msg.event) {
	case IM::rpc_init:

		if (!folder.config["rpc"]["enabled"]) break;

		rpc_loading();

		break;
	case IM::rpc_clear:

		Discord_ClearPresence();

		break;
	case IM::rpc_update: {
		try {
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

			Discord_UpdatePresence(&presence);
		}
		catch (nlohmann::json::type_error e) {
			clog::error << "Unable to parse arguments: " << e.what() << clog::endl;
		}

	} break;
	case IM::account_password: {
		JSMessage res(msg.args[0].get<int>());
		std::string dec;

		if (!accounts.data.contains(msg.args[1])) res.args[1] = "Account doesn't exist";
		else if (!accounts.decrypt(accounts.data[msg.args[1]].password, dec)) res.args[1] = "Unknown";
		else res.args[0] = dec;

		if (!res.send(window.webview)) clog::error << "Unable to send " << res.dump() << clog::endl;

	} break;
	case IM::account_remove: {

		accounts.data.erase(msg.args[0]);
		accounts.save();

		JSMessage res(IM::account_regen);
		res.args.push_back(accounts.dump());
		if (!res.send(window.webview)) clog::error << "Unable to send " << res.dump() << clog::endl;

	} break;
	case IM::account_set: {

		std::string name = msg.args[0];
		Account& account = accounts.data[name];
		account.color = msg.args[1];
		account.order = msg.args[2];
		accounts.save();

		JSMessage res(IM::account_regen);
		res.args.push_back(accounts.dump());
		if (!res.send(window.webview)) clog::error << "Unable to send " << res.dump() << clog::endl;

	} break;
	//and creation
	case IM::account_set_password: {

		std::string enc;
		if (accounts.encrypt(msg.args[1], enc)) {
			Account account;
			account.color = msg.args[2];
			account.order = msg.args[3];
			account.password = enc;
			accounts.data[msg.args[0]] = account;

			accounts.save();

			JSMessage res(IM::account_regen);
			res.args.push_back(accounts.dump()); 
			if (!res.send(window.webview)) clog::error << "Unable to send " << res.dump() << clog::endl;
		}

	} break;
	case IM::account_list: {

		JSMessage res(msg.args[0].get<int>());
		res.args.push_back(accounts.dump());
		if (!res.send(window.webview)) clog::error << "Unable to send " << res.dump() << clog::endl;
		
	} break;

	default:
		return false;
	}

	return true;
}

void Client::install_runtimes() {
	if (MessageBox(NULL, L"You are missing runtimes. Install the WebView2 Runtime?", title, MB_YESNO) == IDYES) {
		WebView2Installer::Error error;
		MessageBox(NULL, L"Relaunch the client after installation is complete.", title, MB_OK);
		if (!installer.Install(error)) switch (error) {
		case WebView2Installer::Error::CantOpenProcess:
			clog::error << "CantOpenProcess during WebView2 installation" << clog::endl;
			MessageBox(NULL, (L"Unable to open " + installer.bin + L". You will need to run the exe manually.").c_str(), title, MB_OK);
			break;
		case WebView2Installer::Error::NoBytesDownloaded:
			clog::error << "NoBytesDownloaded during WebView2 installation" << clog::endl;
			MessageBox(NULL, L"Unable to download MicrosoftEdgeWebview2Setup. Relaunch the client then try again.", title, MB_OK);
			break;
		default:
			clog::error << "Unknown error " << (int)error << " during WebView2 installation" << clog::endl;
			MessageBox(NULL, L"An unknown error occurred.", title, MB_OK);
			break;
		}
	}
	else MessageBox(NULL, L"Cannot continue without runtimes. Client will now exit.", title, MB_OK);
}

Client::Client(HINSTANCE h, int c)
	: inst(h)
	, cmdshow(c)
	, updater(version, "https://6ct.github.io", "/serve/updates.json")
	, installer("https://go.microsoft.com", "/fwlink/p/?LinkId=2124703")
	, folder(L"GC++")
	, game(folder, KrunkerWindow::Type::Game, { 0.8, 0.8 }, title, [this]() { listen_navigation(game); }, [this](JSMessage msg) -> bool { return on_message(msg, game); }, []() { PostQuitMessage(EXIT_SUCCESS); })
	, social(folder, KrunkerWindow::Type::Social, { 0.7, 0.7 }, (std::wstring(title) + L": Social").c_str(), [this]() { listen_navigation(social); }, [this](JSMessage msg) -> bool { return on_message(msg, social); })
	, editor(folder, KrunkerWindow::Type::Editor, { 0.7, 0.7 }, (std::wstring(title) + L": Editor").c_str(), [this]() { listen_navigation(editor); }, [this](JSMessage msg) -> bool { return on_message(msg, editor); })
	, scripting(folder, KrunkerWindow::Type::Scripting, { 0.6, 0.6 }, (std::wstring(title) + L": Scripting").c_str(), [this]() { listen_navigation(scripting); }, [this](JSMessage msg) -> bool { return on_message(msg, scripting); })
	, documents(folder, KrunkerWindow::Type::Documents, { 0.4, 0.6 }, (std::wstring(title) + L": Documents").c_str(), [this]() { listen_navigation(documents); }, [this](JSMessage msg) -> bool { return on_message(msg, documents); })
	, accounts(folder)
	, shcore(LoadLibrary(L"api-ms-win-shcore-scaling-l1-1-1.dll"))
{}

bool Client::create() {
	memset(&presence_events, 0, sizeof(presence_events));
	Discord_Initialize(discord_rpc, &presence_events, 1, NULL);
	
	if (!folder.create()) {
		clog::debug << "Error creating folder" << clog::endl;
		MessageBox(NULL, L"Error creating folder. See Error.log.", title, MB_OK);
		return false;
	}
	
	folder.load_config();
	accounts.load();

	if (folder.config["rpc"]["enabled"]) rpc_loading();

	HRESULT coinit = CoInitialize(NULL);
	if (!SUCCEEDED(coinit)) MessageBox(NULL, (L"COM could not be initialized. CoInitialize returned " + Convert::wstring(std::to_string(coinit))).c_str(), title, MB_OK);

	clog::info << "Main initialized" << clog::endl;

	if (!installer.Installed()) {
		install_runtimes();
		return false;
	}

	game.can_fullscreen = true;
	documents.background = RGB(0xFF, 0xFF, 0xFF);

	if (shcore) {
		using dec = decltype(::SetProcessDpiAwareness);
		if (std::function SetProcessDpiAwareness = (dec*)GetProcAddress(shcore, "SetProcessDpiAwareness")) {
			HRESULT sda = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			if (!SUCCEEDED(sda)) clog::error << "SetProcessDpiAwareness returned 0x" << std::hex << HRESULT_CODE(sda) << clog::endl;
		}
		else clog::error << "Unable to get address of SetProcessDpiAwareness" << clog::endl;
	}
	
	switch (game.create(inst, cmdshow, [this]() {
		game.webview->Navigate(L"https://krunker.io");
	})) {
	case KrunkerWindow::Status::Ok: break;
	case KrunkerWindow::Status::MissingRuntime:
		clog::error << "WebView2 installation check failed. Unable to create game window." << clog::endl;
		install_runtimes();
		return false;
	case KrunkerWindow::Status::UserDataExists:
		clog::error << "Unable to create user data folder." << clog::endl;
		game.MessageBox(L"Unable to create the user data folder. Delete the GC++ folder in your documents then relaunch the client.", title);
		break;
	case KrunkerWindow::Status::RuntimeFatal:
		clog::error << "Fatal Edge runtime error occured." << clog::endl;
		game.MessageBox(L"An unknown Edge runtime error occured. Try relaunching the client.", title);;
		break;
	case KrunkerWindow::Status::UnknownError:
	default:
		std::wstringstream sstream;
		sstream << std::hex << game.last_herror;
		
		game.MessageBox(
			(std::wstring(L"An unknown error ocurred during game creation. Create a issue on GitHub ( https://github.com/6ct/clientpp/issues ) and provide the following error details:\n") +
			L"Error code: 0x" + sstream.str()).c_str(), title, MB_ICONERROR);

		return false;
		break;
	};

	// checking updates causes delay
	new std::thread([this]() {
		UpdaterServing serving;
		if (updater.UpdatesAvailable(serving) && MessageBox(game.m_hWnd, L"A new client update is available. Download?", title, MB_YESNO) == IDYES) {
			ShellExecute(game.m_hWnd, L"open", Convert::wstring(serving.url).c_str(), L"", L"", SW_SHOW);
			exit(EXIT_SUCCESS);
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