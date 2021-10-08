// fix to aim freeze:
// hook websocket, reduce amount of packets sent when shooting

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <atlbase.h>
#include <atlwin.h>
#include <atlenc.h>
#include <stdlib.h>
#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
#include <thread>
#include <mutex>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include "WebView2.h"
#include "../Utils/StringUtil.h"
#include "../Utils/IOUtil.h"
#include "../Utils/JSON.h"
#include "../Utils/Base64.h"
#include "../Utils/Uri.h"
#include "./resource.h"
#include "./Log.h"
#include "./Consts.h"
#include "./Updater.h"
#include "./Points.h"
#include "./LoadRes.h"
#include "./ClientFolder.h"
#include "./WebView2Installer.h"

using namespace StringUtil;
using namespace Microsoft::WRL;

class WebViewWindow : public CWindowImpl<WebViewWindow> {
public:
	wil::com_ptr<ICoreWebView2Controller> control;
	wil::com_ptr<ICoreWebView2> webview;
	wil::com_ptr<ICoreWebView2Environment> env;
	std::wstring title;
	ClientFolder folder;
	Vector2 scale;
	HINSTANCE get_hinstance() {
		return (HINSTANCE)GetWindowLong(GWL_HINSTANCE);
	}
	bool create_window(HINSTANCE inst, int cmdshow) {
		if (folder.config["window"]["meta"]["replace"].get<bool>())
			title = Convert::wstring(folder.config["window"]["meta"]["title"].get<std::string>());

		Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);

		SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(0, 0, 0)));

		if (folder.config["window"]["meta"]["replace"].get<bool>())
			SetIcon((HICON)LoadImage(inst, Convert::wstring(folder.config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		else SetIcon(LoadIcon(inst, MAKEINTRESOURCE(MAINICON)));

		Vector2 scr_pos;
		Vector2 scr_size;

		if (monitor_data(scr_pos, scr_size)) {
			Rect2D r;

			r.width = long(scr_size.x * scale.x);
			r.height = long(scr_size.y * scale.y);

			r.x = long(scr_pos.x + ((scr_size.x - r.width) / 2));
			r.y = long(scr_pos.y + ((scr_size.y - r.height) / 2));

			SetWindowPos(NULL, r.get(), 0);
		}
		else ResizeClient(700, 500);

		ShowWindow(cmdshow);
		UpdateWindow();

		open = true;
		
		return true;
	}
	void create_webview(std::wstring cmdline, std::function<void()> callback) {
		auto options = Make<CoreWebView2EnvironmentOptions>();

		options->put_AdditionalBrowserArguments(cmdline.c_str());
		
		CreateCoreWebView2EnvironmentWithOptions(nullptr, (folder.directory + folder.p_profile).c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this, callback](HRESULT result, ICoreWebView2Environment* envp) -> HRESULT {
				if (envp == nullptr) {
					LOG_ERROR("Env was nullptr");
					return S_FALSE;
				}
				env = envp;
				env->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this, callback](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
					if (controller == nullptr) {
						LOG_ERROR("Controller was nullptr");
						return S_FALSE;
					}

					control = controller;
					control->get_CoreWebView2(&webview);

					callback();
					return S_OK;
				}).Get());

				return S_OK;
		}).Get());
	}
	COREWEBVIEW2_COLOR colorref2webview(COLORREF color) {
		COREWEBVIEW2_COLOR wv;
		wv.R = GetRValue(color);
		wv.G = GetGValue(color);
		wv.B = GetBValue(color);
		wv.A = 255;
		return wv;
	}
	RECT windowed;
	bool fullscreen = false;
	bool enter_fullscreen() {
		if (fullscreen) return false;

		RECT screen;

		if (!monitor_data(screen)) return false;

		GetClientRect(&windowed);
		ClientToScreen(&windowed);

		SetWindowLongPtr(GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
		SetWindowLongPtr(GWL_STYLE, WS_POPUP | WS_VISIBLE);

		SetWindowPos(0, &screen, SWP_SHOWWINDOW);
		ShowWindow(SW_MAXIMIZE);

		resize_wv();

		return fullscreen = true;
	}
	bool exit_fullscreen() {
		if (!fullscreen) return false;

		SetWindowLongPtr(GWL_EXSTYLE, WS_EX_LEFT);
		SetWindowLongPtr(GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		SetWindowPos(HWND_NOTOPMOST, RECT_ARGS(windowed), SWP_SHOWWINDOW);
		ShowWindow(SW_RESTORE);

		fullscreen = false;

		resize_wv();

		return true;
	}
	bool resize_wv() {
		if (control == nullptr) return false;

		RECT bounds;
		GetClientRect(&bounds);
		control->put_Bounds(bounds);

		return true;
	}
	bool monitor_data(RECT& rect) {
		HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info;
		info.cbSize = sizeof(info);

		if (!GetMonitorInfo(monitor, &info))return LOG_ERROR("Can't get monitor info"), false;

		rect = info.rcMonitor;

		return true;
	}
	bool monitor_data(Vector2& pos, Vector2& size) {
		RECT r;

		if (!monitor_data(r))return false;

		pos.x = r.left;
		pos.y = r.top;

		size.x = r.right - r.left;
		size.y = r.bottom - r.top;

		return true;
	}
	bool open = false;
	WebViewWindow(ClientFolder& f, Vector2 s, std::wstring t) : title(t), folder(f), scale(s) {}
	~WebViewWindow() {
		// app shutdown
		// assertion error
		if (::IsWindow(m_hWnd)) DestroyWindow();
	}
	JSON runtime_data() {
		JSON data = JSON::object();

		struct Search {
			std::wstring dir;
			std::wstring filter;
			JSON& obj;
		};

		for (Search search : std::vector<Search>{
			{folder.p_styles, L"*.css", data["css"] = JSON::object()},
			{folder.p_scripts, L"*.js", data["js"] = JSON::object()},
			})
			for (IOUtil::WDirectoryIterator it(folder.directory + search.dir, search.filter); ++it;) {
				std::string buffer;

				if (IOUtil::read_file(it.path().c_str(), buffer))
					search.obj[Convert::string(it.file()).c_str()] = buffer;
			}

		std::string css_client;
		if (load_resource(CSS_CLIENT, css_client)) data["css"]["Client/Client.css"] = css_client;

		data["config"] = folder.config;

		return data;
	}
	LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
		return resize_wv();
	}
	LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
		open = false;
		return true;
	}
	BEGIN_MSG_MAP(KrunkerWindow)
		MESSAGE_HANDLER(WM_SIZE, on_resize)
		MESSAGE_HANDLER(WM_DESTROY, on_destroy)
	END_MSG_MAP()
	virtual void get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) {}
	virtual void create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {}
	virtual void on_dispatch() {}
};

template<class T>
class KrunkerWindow : public WebViewWindow {
public:
	std::vector<std::wstring> blocked_hosts{
		L"cookie-cdn.cookiepro.com",
		L"googletagmanager.com",
		L"googlesyndication.com",
		L"a.pub.network",
		L"paypalobjects.com",
		L"doubleclick.net",
		L"adinplay.com",
		L"syndication.twitter.com"
	};
	std::vector<JSON> post;
	std::mutex mtx;
	std::wstring og_title;
	// https://peter.sh/experiments/chromium-command-line-switches/
	std::wstring cmdline() {
		std::vector<std::wstring> cmds = {
			// L"--profile-directory=Profile",
			L"--force-dark-mode",
			L"--autoplay-policy=no-user-gesture-required",
			// L"disable-background-timer-throttling"
		};

		if (folder.config["client"]["uncap_fps"].get<bool>()) {
			cmds.push_back(L"--disable-frame-rate-limit");
			cmds.push_back(L"--disable-gpu-vsync");
		}

		std::wstring cmdline;
		bool first = false;

		for (std::wstring cmd : cmds) {
			if (first) first = false;
			else cmdline += L" ";

			cmdline += cmd;
		}

		return cmdline;
	}
	void KrunkerEvents() {
		EventRegistrationToken token;

		std::string bootstrap;
		if (load_resource(JS_BOOTSTRAP, bootstrap)) webview->AddScriptToExecuteOnDocumentCreated(Convert::wstring(bootstrap).c_str(), nullptr);
		else LOG_ERROR("Error loading bootstrapper");

		webview->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
		webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
			LPWSTR mpt;
			args->TryGetWebMessageAsString(&mpt);

			if (mpt) {
				JSON message = JSON::parse(Convert::string(mpt));
				std::string event = message[0].get<std::string>();

				if (event == "send webpack") {
					JSON response = JSON::array();

					response[0] = "eval webpack";

					std::string js_webpack = "throw Error('Failure loading Webpack.js');";
					load_resource(JS_WEBPACK, js_webpack);
					std::string js_webpack_map;
					load_resource(JS_WEBPACK_MAP, js_webpack_map);

					js_webpack += "\n//# sourceMappingURL=data:application/json;base64," + Base64::Encode(js_webpack_map);

					response[1] = js_webpack;
					response[2] = runtime_data();

					sender->PostWebMessageAsJson(Convert::wstring(response.dump()).c_str());
				}
				else if (event == "save config") {
					folder.config = message[1];
					folder.save_config();
				}
				else if (event == "open") {
					std::wstring open;

					if (message[1] == "root")open = folder.directory;
					else if (message[1] == "scripts") open = folder.directory + folder.p_scripts;
					else if (message[1] == "styles") open = folder.directory + folder.p_styles;
					else if (message[1] == "swapper") open = folder.directory + folder.p_swapper;
					else if (message[1] == "url") open = Convert::wstring(message[2].get<std::string>());

					ShellExecute(NULL, L"open", open.c_str(), L"", L"", SW_SHOW);
				}
				else if (event == "relaunch") {
					// if (::IsWindow(m_hWnd)) DestroyWindow();
					// https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller?view=webview2-1.0.992.28#close
					control->Close();

					((T*)this)->call_create_webview([]() {});
				}
				else if (event == "fullscreen") {
					if (folder.config["client"]["fullscreen"].get<bool>()) enter_fullscreen();
					else exit_fullscreen();
				}
				else if (event == "update meta") {
					title = Convert::wstring(folder.config["window"]["meta"]["title"].get<std::string>());
					SetIcon((HICON)LoadImage(get_hinstance(), Convert::wstring(folder.config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
					SetWindowText(title.c_str());
				}
				else if (event == "revert meta") {
					title = og_title;
					SetIcon(LoadIcon(get_hinstance(), MAKEINTRESOURCE(MAINICON)));
					SetWindowText(title.c_str());
				}
				else if (event == "reload config") {
					folder.load_config();
				}
				else if (event == "browse file") new std::thread([this](JSON message) {
					wchar_t filename[MAX_PATH];

					OPENFILENAME ofn;
					ZeroMemory(&filename, sizeof(filename));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
					// std::wstring filters;
					std::wstring title = Convert::wstring(message[2]);
					// std::vector<wchar_t> filters;
					std::wstring filters;
					for (JSON value : message[3]) {
						std::string label = value[0];
						std::string filter = value[1];

						label += " (" + filter + ")";

						filters += Convert::wstring(label);
						filters += L'\0';
						filters += Convert::wstring(filter);
						filters += L'\0';
					}

					// L"Icon Files\0*.ico\0Any File\0*.*\0"
					// filters is terminated by 2 null characters
					// each filter is terminated by 1 null character
					ofn.lpstrFilter = filters.c_str();
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrTitle = title.c_str();
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

					JSON response = JSON::array();
					response[0] = message[1];

					if (GetOpenFileName(&ofn)) {
						response[1] = Convert::string(filename);
						response[2] = false;
					}
					else {
						response[2] = true;
					}

					mtx.lock();
					post.push_back(response);
					mtx.unlock();
				}, message);
			}
			else LOG_ERROR("Recieved invalid message");

			return S_OK;
		}).Get(), &token);

		webview->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
			LPWSTR sender_uriptr;
			sender->get_Source(&sender_uriptr);

			ICoreWebView2WebResourceRequest* request;
			args->get_Request(&request);
			LPWSTR uriptr;
			request->get_Uri(&uriptr);
			Uri uri(uriptr);

			if (uri.HostEquals(L"krunker.io")) {
				std::wstring swap = folder.directory + folder.p_swapper + uri.pathname();

				if (IOUtil::file_exists(swap)) {
					LOG_INFO("Swapping " << Convert::string(uri.pathname()));
					// Create an empty IStream:
					IStream* stream;

					if (SHCreateStreamOnFileEx(swap.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, 0, &stream) == S_OK) {
						wil::com_ptr<ICoreWebView2WebResourceResponse> response;
						env->CreateWebResourceResponse(stream, 200, L"OK", L"access-control-allow-origin: https://krunker.io\naccess-control-expose-headers: Content-Length, Content-Type, Date, Server, Transfer-Encoding, X-GUploader-UploadID, X-Google-Trace", &response);
						args->put_Response(response.get());
					}
					else LOG_ERROR("Error creating IStream on swap: " << Convert::string(swap));
				}
			}
			else for (std::wstring test : blocked_hosts) if (uri.HostEquals(test)) {
				wil::com_ptr<ICoreWebView2WebResourceResponse> response;
				env->CreateWebResourceResponse(nullptr, 403, L"Blocked", L"Content-Type: text/plain", &response);
				args->put_Response(response.get());
				break;
			}

			return S_OK;
			}).Get(), &token);
	}
	void create(HINSTANCE inst, int cmdshow, std::function<void()> callback) override {
		create_window(inst, cmdshow);
		((T*)this)->call_create_webview(callback);
	}
	void get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) override {
		if (!open) create(inst, cmdshow, [this, callback]() {
			callback(true);
			});
		else callback(false);
	}
	void on_dispatch() override {
		mtx.lock();
		for (JSON message : post)
			webview->PostWebMessageAsJson(Convert::wstring(message.dump()).c_str());

		post.clear();
		mtx.unlock();
	}
	KrunkerWindow(ClientFolder& folder , Vector2 scale, std::wstring title) : WebViewWindow(folder, scale, title), og_title(title) {}
};

class SocialWindow : public KrunkerWindow<SocialWindow> {
public:
	SocialWindow(ClientFolder& f) : KrunkerWindow(f, { 0.4, 0.6 }, L"Guru Client++") {}
	void call_create_webview(std::function<void()> callback) {
		create_webview(cmdline(), [this, callback]() {
			ICoreWebView2Settings* settings;
			webview->get_Settings(&settings);
			settings->put_IsScriptEnabled(TRUE);
			settings->put_AreDefaultScriptDialogsEnabled(TRUE);
			settings->put_IsWebMessageEnabled(TRUE);
#if _DEBUG != 1
			settings->put_AreDevToolsEnabled(FALSE);
#else
			webview->OpenDevToolsWindow();
#endif
			settings->put_IsZoomControlEnabled(FALSE);

			resize_wv();

			KrunkerEvents();

			webview->Navigate(L"https://krunker.io/social.html");

			LOG_INFO("Social window created");

			callback();
		});
	}
};

class EditorWindow : public KrunkerWindow<EditorWindow> {
public:
	EditorWindow(ClientFolder& f) : KrunkerWindow(f, { 0.4, 0.6 }, L"Social") {}
	void call_create_webview(std::function<void()> callback) {
		create_webview(cmdline(), [this, callback]() {
			ICoreWebView2Settings* settings;
			webview->get_Settings(&settings);
			settings->put_IsScriptEnabled(TRUE);
			settings->put_AreDefaultScriptDialogsEnabled(TRUE);
			settings->put_IsWebMessageEnabled(TRUE);
#if _DEBUG != 1
			settings->put_AreDevToolsEnabled(FALSE);
#else
			webview->OpenDevToolsWindow();
#endif
			settings->put_IsZoomControlEnabled(FALSE);

			resize_wv();

			KrunkerEvents();

			webview->Navigate(L"https://krunker.io/editor.html");

			LOG_INFO("Game window created");

			callback();
		});
	}
};

class GameWindow : public KrunkerWindow<GameWindow> {
public:
	void call_create_webview(std::function<void()> callback) {
		create_webview(cmdline(), [this, callback]() {
			ICoreWebView2Settings* settings;
			webview->get_Settings(&settings);
			settings->put_IsScriptEnabled(TRUE);
			settings->put_AreDefaultScriptDialogsEnabled(TRUE);
			settings->put_IsWebMessageEnabled(TRUE);
#if _DEBUG != 1
			settings->put_AreDevToolsEnabled(FALSE);
#else
			webview->OpenDevToolsWindow();
#endif
			settings->put_IsZoomControlEnabled(FALSE);

			resize_wv();
			
			KrunkerEvents();

			webview->Navigate(L"https://krunker.io/");

			callback();
		});
	}
	GameWindow(ClientFolder& f) : KrunkerWindow(f, { 0.8, 0.8 }, L"Guru Client++") {}
};

class Main {
private:
	std::wstring title = L"Guru Client++";
	ClientFolder folder;
	Updater updater;
	WebView2Installer installer;
	GameWindow game;
	SocialWindow social;
	EditorWindow editor;
	HINSTANCE inst;
	int cmdshow;
	bool navigation_cancelled(ICoreWebView2* sender, Uri uri) {
		bool cancel = false;
		std::wstring uhost = uri.host();
		WebViewWindow* send = 0;

		if (uhost == L"krunker.io") {
			if(uri.pathname() == L"/"){
				send = &game;
			}
			else if (uri.pathname() == L"/social.html") {
				send = &social;
			}
			else if (uri.pathname() == L"/editor.html") {
				send = &editor;
			}
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
	void listen_navigation(WebViewWindow& window) {
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
		, folder(L"GC++")
		, updater(CLIENT_VERSION, "https://y9x.github.io", "/userscripts/serve.json")
		, installer("https://go.microsoft.com", "/fwlink/p/?LinkId=2124703")
		, game(folder)
		, social(folder)
		, editor(folder)
	{
		CoInitialize(NULL);

		std::string update_url;
		if (updater.UpdatesAvailable(update_url) && ::MessageBox(NULL, L"A new client update is available. Download?", title.c_str(), MB_YESNO) == IDYES) {
			ShellExecute(NULL, L"open", Convert::wstring(update_url).c_str(), L"", L"", SW_SHOW);
			return;
		}

		if (!installer.Installed()) {
			if (::MessageBox(NULL, L"You are missing runtimes. Do you wish to install WebView2 Runtime?", title.c_str(), MB_YESNO) == IDYES) {
				WebView2Installer::Error error;
				if (installer.Install(error))
					::MessageBox(NULL, L"Relaunch the client after installation is complete.", title.c_str(), MB_OK);
				else switch (error) {
				case WebView2Installer::Error::CantOpenProcess:
					::MessageBox(NULL, (L"Couldn't open " + installer.bin + L". You will need to run the exe manually.").c_str(), title.c_str(), MB_OK);
					break;
				}
			}
			else ::MessageBox(NULL, L"Cannot continue without runtimes, quitting...", title.c_str(), MB_OK);
			return;
		}

		game.create(inst, cmdshow, [this]() {
			listen_navigation(game);
		});

		MSG msg;
		BOOL ret;

		while (game.open && (ret = GetMessage(&msg, 0, 0, 0))) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			game.on_dispatch();
		}
	}
};

int APIENTRY WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, _In_ LPSTR cmdline, _In_ int nCmdShow) {
#if WILL_LOG == 1
	SetConsoleCtrlHandler(0, true);
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		freopen("CONIN$", "r", stdin);

		std::cout.clear();
		std::cin.clear();
	}
	else MessageBox(NULL, L"Failure attatching console", L"Debugger", MB_OK);
#endif

	Main m(hInstance, nCmdShow);
	return EXIT_SUCCESS;
}