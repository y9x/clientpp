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

class Window : public CWindowImpl<Window> {
private:
	std::wstring title = L"Guru Client++";
	std::wstring og_title = title;
	std::vector<std::wstring> blocked_hosts {
		L"cookie-cdn.cookiepro.com",
		L"googletagmanager.com",
		L"googlesyndication.com",
		L"a.pub.network",
		L"paypalobjects.com",
		L"doubleclick.net",
		L"adinplay.com",
		L"syndication.twitter.com"
	};
	std::mutex mtx;
	ClientFolder folder;
	Updater updater;
	WebView2Installer installer;
	wil::com_ptr<ICoreWebView2Controller> game_control;
	wil::com_ptr<ICoreWebView2> game;
	std::vector<JSON> game_post;
	Vector2 game_size = Vector2(0.8, 0.8);
	JSON runtime_data() {
		JSON data = JSON::object();

		struct Search {
			std::wstring dir;
			std::wstring filter;
			JSON& obj;
		};

		for (Search search : std::vector<Search> {
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
	COREWEBVIEW2_COLOR colorref2webview(COLORREF color) {
		COREWEBVIEW2_COLOR wv;
		wv.R = GetRValue(color);
		wv.G = GetGValue(color);
		wv.B = GetBValue(color);
		wv.A = 255;
		return wv;
	}
	// https://peter.sh/experiments/chromium-command-line-switches/
	// 
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
	HINSTANCE hinst;
	RECT windowed;
	int cmdshow;
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
	bool cancel_navigation(ICoreWebView2* sender, Uri uri) {
		bool cancel = false;
		std::wstring uhost = uri.host();
		ICoreWebView2* send_to = 0;

		if (uhost == L"krunker.io") {
			send_to = game.get();
		}
		else {
			cancel = true;
			ShellExecute(NULL, L"open", uri.href.c_str(), L"", L"", SW_SHOW);
		}

		if (send_to && send_to != sender) {
			cancel = true;
			send_to->Navigate(uri.href.c_str());
		}

		return cancel;
	}
	// todo: open non-krunker urls in shell, create windows for social and editor
	void init_webview2() {
		auto options = Make<CoreWebView2EnvironmentOptions>();

		std::wstring cm = cmdline();

		options->put_AdditionalBrowserArguments(cm.c_str());
		
		CreateCoreWebView2EnvironmentWithOptions(nullptr, (folder.directory + folder.p_profile).c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
			env->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([env, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
				if (controller == nullptr) {
					LOG_ERROR("Controller was nullptr");
					return S_FALSE;
				}

				game_control = controller;
				game_control->get_CoreWebView2(&game);

				wil::com_ptr<ICoreWebView2Controller2> controller2;
				controller2 = game_control.query<ICoreWebView2Controller2>();
				if (controller2) controller2->put_DefaultBackgroundColor(colorref2webview(RGB(0, 0, 0)));

				ICoreWebView2Settings* settings;
				game->get_Settings(&settings);
				settings->put_IsScriptEnabled(TRUE);
				settings->put_AreDefaultScriptDialogsEnabled(TRUE);
				settings->put_IsWebMessageEnabled(TRUE);
#if _DEBUG != 1
				settings->put_AreDevToolsEnabled(FALSE);
#else
				game->OpenDevToolsWindow();
#endif
				settings->put_IsZoomControlEnabled(FALSE);

				resize_wv();

				game_control->put_ZoomFactor(1);

				if (folder.config["client"]["fullscreen"].get<bool>()) enter_fullscreen();

				EventRegistrationToken token;

				std::string bootstrap;
				if (load_resource(JS_BOOTSTRAP, bootstrap)) game->AddScriptToExecuteOnDocumentCreated(Convert::wstring(bootstrap).c_str(), nullptr);
				else LOG_ERROR("Error loading bootstrapper");
				
				game->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>([env, this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
					LPWSTR urip;
					args->get_Uri(&urip);
					if (cancel_navigation(sender, urip)) {
						args->put_Cancel(true);
					}
					
					return S_OK;
				}).Get(), &token);

				game->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>([env, this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
					LPWSTR urip;
					args->get_Uri(&urip);
					if (cancel_navigation(sender, urip)) {
						args->put_Handled(true);
					} else args->put_NewWindow(sender);
					
					/*Uri uri(urip);
					LOG_INFO("NEW WINDOW: " << Convert::string(uri.href));
					// args->put_NewWindow(sender);
					game->Navigate(uri.href.c_str());
					*/
					return S_OK;
				}).Get(), &token);

				// COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SCRIPT
				// recieve all requests for resource swapper
				game->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

				game->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([env, this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
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
							game_control->Close();

							init_webview2();
							// init();
						}
						else if (event == "fullscreen") {
							if (folder.config["client"]["fullscreen"].get<bool>()) enter_fullscreen();
							else exit_fullscreen();
						}
						else if (event == "update meta") {
							title = Convert::wstring(folder.config["window"]["meta"]["title"].get<std::string>());
							SetIcon((HICON)LoadImage(hinst, Convert::wstring(folder.config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
							SetWindowText(title.c_str());
						}
						else if (event == "revert meta") {
							title = og_title;
							SetIcon(LoadIcon(hinst, MAKEINTRESOURCE(MAINICON)));
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

								std::cout << label << " : " << filter << std::endl;

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
							game_post.push_back(response);
							mtx.unlock();
						}, message);
					}
					else LOG_ERROR("Recieved invalid message");

					return S_OK;
				}).Get(), &token);

				game->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([env, this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
					// memory doesnt need to be allocated/deallocated since it assigns to a pointer...
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

				game->Navigate(L"https://krunker.io/");

				return S_OK;
			}).Get());

			return S_OK;
		}).Get());
	}
	bool resize_wv() {
		if (game_control == nullptr) return false;

		RECT bounds;
		GetClientRect(&bounds);
		game_control->put_Bounds(bounds);

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
public:
	Window(HINSTANCE h, int c)
		: hinst(h)
		, cmdshow(c)
		, folder(L"GC++")
		, updater(CLIENT_VERSION, "https://y9x.github.io", "/userscripts/serve.json")
		, installer("https://go.microsoft.com", "/fwlink/p/?LinkId=2124703")
	{
		if (!installer.Installed()){
			if(::MessageBox(NULL, L"You are missing runtimes. Do you wish to install WebView2 Runtime?", title.c_str(), MB_YESNO) == IDYES) {
				WebView2Installer::Error error;
				if (installer.Install(error))
					::MessageBox(NULL, L"Relaunch the client after installation is complete.", title.c_str(), MB_OK);
				else {
					switch (error) {
					case WebView2Installer::Error::CantOpenProcess:
					
						::MessageBox(NULL, (L"Couldn't open " + installer.bin + L". You will need to run the exe manually.").c_str(), title.c_str(), MB_OK);
						
						break;
					}
				}
			}
			else ::MessageBox(NULL, L"Cannot continue without runtimes, quitting...", title.c_str(), MB_OK);

			PostQuitMessage(EXIT_SUCCESS);
			return;
		}

		std::string update_url;
		if (updater.UpdatesAvailable(update_url) && ::MessageBox(NULL, L"A new client update is available. Download?", title.c_str(), MB_YESNO) == IDYES) {
			ShellExecute(NULL, L"open", Convert::wstring(update_url).c_str(), L"", L"", SW_SHOW);
			PostQuitMessage(EXIT_SUCCESS);
			return;
		}
		
		if (folder.config["window"]["meta"]["replace"].get<bool>())
			title = Convert::wstring(folder.config["window"]["meta"]["title"].get<std::string>());

		Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);

		SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(0, 0, 0)));

		if (folder.config["window"]["meta"]["replace"].get<bool>())
			SetIcon((HICON)LoadImage(hinst, Convert::wstring(folder.config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		else SetIcon(LoadIcon(hinst, MAKEINTRESOURCE(MAINICON)));

		Vector2 scr_pos;
		Vector2 scr_size;

		if (monitor_data(scr_pos, scr_size)) {
			Rect2D r;
			
			r.width = long(scr_size.x * game_size.x);
			r.height = long(scr_size.y * game_size.y);

			r.x = long(scr_pos.x + ((scr_size.x - r.width) / 2));
			r.y = long(scr_pos.y + ((scr_size.y - r.height) / 2));

			SetWindowPos(NULL, r.get(), 0);
		}
		else ResizeClient(700, 500);

		CoInitialize(NULL);
		init_webview2();

		ShowWindow(cmdshow);
		UpdateWindow();

		MSG msg;
		BOOL ret;

		LOG_INFO("Client Initialized");

		while (ret = GetMessage(&msg, 0, 0, 0)) {
			mtx.lock();
			for (JSON message : game_post) 
				game->PostWebMessageAsJson(Convert::wstring(message.dump()).c_str());

			game_post.clear();
			mtx.unlock();

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	~Window() {
		// app shutdown
		// assertion error
		if (::IsWindow(m_hWnd)) DestroyWindow();
	}
	LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
		return resize_wv();
	}
	LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
		PostQuitMessage(EXIT_SUCCESS);
		return true;
	}
	BEGIN_MSG_MAP(Window)
		MESSAGE_HANDLER(WM_SIZE, on_resize)
		MESSAGE_HANDLER(WM_DESTROY, on_destroy)
	END_MSG_MAP()
};

/*class LApp : public CAtlExeModuleT<LApp> {
public:
	static HRESULT InitializeCom() {
		CoInitialize(NULL);
		return S_OK;
	}
};

LApp app;*/

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

	Window win(hInstance, nCmdShow);
	return EXIT_SUCCESS;
}