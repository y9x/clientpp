#define WILL_LOG 1

#define VERSION 1

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <windows.h>
#include <chrono>
#include <ctime>

#if WILL_LOG == 1
#define LOG_COUT std::cout
#else
class FileCout : private std::streambuf, public std::ostream {
private:
	std::string buffer;
	int overflow(int c) override {
		if (c == EOF) return 0;
		
		buffer += c;
		
		if (c == '\n') {
			FILE* handle = _wfopen(path.c_str(), L"a");
			if (handle) {
				fwrite(buffer.data(), sizeof(char), buffer.size(), handle);
				fclose(handle);
			}
			buffer.erase();
		}

		return 0;
	}
public:
	std::wstring path;
	FileCout(std::wstring p) : std::ostream(this), path(p) {}
};

FileCout fc(L"./TmpLogs.txt");
#define LOG_COUT fc
#endif

std::string create_log_badge(std::string type) {
	std::string result = "[" + type + "]";

#if WILL_LOG == 0
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string ts = std::ctime(&now);
	ts.pop_back();
	result += " [" + ts + "]";
#endif

	result += ": ";

	return result;
}

// #define LOG(data) LOG_COUT << data << '\n'
#define LOG_INFO(data) LOG_COUT << create_log_badge("Info") << data << '\n'
#define LOG_WARN(data) LOG_COUT << create_log_badge("Warning") << data << '\n'
#define LOG_ERROR(data) LOG_COUT << create_log_badge("Error") << data << '\n'

#define RECT_ARGS(r) r.left, r.top, r.right - r.left, r.bottom - r.top

#include <vector>
#include <atlbase.h>
#include <atlwin.h>
#include <atlenc.h>
#include <stdlib.h>
#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include "WebView2.h"
#include "../Utils/StringUtil.h"
#include "../Utils/IOUtil.h"
#include "../Utils/JSON.h"
#include "../Utils/Base64.h"
#include "./Socket.h"
#include "resource.h"
#include <thread>
#include <thread>
#include <mutex>

using namespace StringUtil;
using namespace Microsoft::WRL;

// manage RECT and retrieve unique pointer on stack
class Rect2D {
private:
	RECT rect;
public:
	LONG x = 0;
	LONG y = 0;
	LONG width = 0;
	LONG height = 0;
	Rect2D(LONG lX, LONG lY, LONG lWidth, LONG lHeight) : x(lX), y(lY), width(lWidth), height(lHeight) {}
	RECT* get() {
		rect.left = x;
		rect.top = y;
		rect.right = x + width;
		rect.bottom = y + height;
		return &rect;
	}
};

bool load_resource(int resource, std::string& string) {
	bool ret = false;
	HRSRC src = FindResource(NULL, MAKEINTRESOURCE(resource), RT_RCDATA);

	if (src != NULL) {
		HGLOBAL header = LoadResource(NULL, src);
		if (header != NULL) {
			char* data = (char*)LockResource(header);
			
			if (data != NULL) {
				string = data;
				string.resize(SizeofResource(0, src));
				ret = true;
			}

			UnlockResource(header);
		}

		FreeResource(header);
	}

	return ret;
}

class ClientFolder {
public:
	std::wstring directory;
	std::wstring p_profile = L"\\Profile";
	std::wstring p_scripts = L"\\Scripts";
	std::wstring p_styles = L"\\Styles";
	std::wstring p_swapper = L"\\Swapper";
	std::wstring p_config = L"\\Config.json";
	std::wstring p_icon = L"\\Guru.ico";
	std::wstring p_log = L"\\Log.txt";
	JSON default_config = JSON::object();
	JSON config = JSON::object();
	ClientFolder(std::wstring n) : name(n) {
		directory = _wgetenv(L"USERPROFILE");
		directory += L"\\Documents\\" + name;

		if (OVR(CreateDirectory(directory.c_str(), NULL))) {
			LOG_INFO("Created " << Convert::string(directory));
			
			HRSRC src = FindResource(NULL, MAKEINTRESOURCE(ICON_GURU), RT_RCDATA);

			if (src != NULL) {
				HGLOBAL header = LoadResource(NULL, src);
				if (header != NULL) {
					void* data = (char*)LockResource(header);

					if (data != NULL) {
						HANDLE file = CreateFile(
							(directory + p_icon).c_str(),     // Filename
							GENERIC_WRITE,          // Desired access
							FILE_SHARE_READ,        // Share mode
							NULL,                   // Security attributes
							CREATE_NEW,             // Creates a new file, only if it doesn't already exist
							FILE_ATTRIBUTE_NORMAL,  // Flags and attributes
							NULL);                  // Template file handle

						if (file != INVALID_HANDLE_VALUE) {
							DWORD size = SizeofResource(0, src);

							DWORD bytesWritten;
							WriteFile(
								file, // Handle to the file
								data, // Buffer to write
								size, // Buffer size
								&bytesWritten,    // Bytes written
								nullptr);         // Overlapped

							CloseHandle(file);
							LOG_INFO("Created " << Convert::string(directory + p_icon));
						}
					}
					UnlockResource(header);
				}
				
				FreeResource(header);
			}

			for (std::wstring sdir : directories) {
				if (OVR(CreateDirectory((directory + sdir).c_str(), NULL))) LOG_INFO("Created " << Convert::string(directory + sdir));
				else error_creating = true;
			}

#if WILL_LOG != 1
			fc.path = directory + p_log;
#endif
		}
		else LOG_ERROR("Creation"), error_creating = true;

		std::string config_buffer;
		if (load_resource(JSON_CONFIG, config_buffer)) default_config = JSON::parse(config_buffer);

		if (error_creating)LOG_ERROR("Had an error creating directories");
		else load_config();
	}
	bool load_config() {
		std::string config_buffer;

		IOUtil::read_file(directory + p_config, config_buffer);

		JSON new_config = JSON::object();

		try {
			new_config = JSON::parse(config_buffer);
		}
		catch (JSON::exception err) {
			new_config = default_config;
			// JSON::parse(config_buffer);
			// non-unicode, might lose data
			new_config["window"]["icon"] = Convert::string(directory + p_icon);
			save_config();
		}

		config = traverse_copy(new_config, default_config, default_config);

		return true;
	}
	JSON traverse_copy(JSON value, JSON match, JSON preset_result = JSON::object()) {
		JSON result;
		if (value.is_object()) {
			result = preset_result; // JSON::object();
			for (auto [skey, svalue] : value.items()) {
				if (match.contains(skey)) {
					result[skey] = traverse_copy(svalue, match[skey]);
				}
				// selse std::cout << skey << " from " << value << " does not match " << match << std::endl;;
			}
		}
		else {
			result = value;
		}

		return result;
	}
	bool save_config() {
		LOG_INFO("Wrote config");
		return IOUtil::write_file(directory + p_config, config.dump(1, '\t'));
	}
private:
	std::wstring name;
	std::vector<std::wstring> directories{ p_scripts, p_styles, p_swapper, p_profile };
	int last_error = 0;
	// true if the result of CreateDirectory is nonzero or if GetLastError equals ERROR_ALREADY_EXISTS, otherwise false
	bool OVR(int result) {
		if (result != 0)return true;
		else if (GetLastError() == ERROR_ALREADY_EXISTS)return true;
		else return false;
	}
	bool error_creating = false;
};

/*template<class T>
class TaskPtr {
	T ptr;
public:
	bool valid() {
		return ptr != 0;
	}
	T& get() {
		return ptr;
	}
	~TaskPtr() {
		CoTaskMemFree(ptr);
	}
};*/

struct Vector2 {
	long x;
	long y;
	bool operator = (POINT p) {
		x = p.x;
		y = p.y;
		return true;
	}
	operator POINT () {
		POINT point;
		point.x = x;
		point.y = y;
		return point;
	}
};

class Window : public CWindowImpl<Window> {
private:
	std::wstring title = L"Guru Client++";
	std::wstring og_title = title;
	std::vector<std::wstring> blocked_script_hosts {
		L"cookie-cdn.cookiepro.com",
		L"googletagmanager.com",
		L"googlesyndication.com",
		L"a.pub.network",
		L"paypalobjects.com",
		L"doubleclick.net",
		L"adinplay.com",
		L"syndication.twitter.com"
	};
	ClientFolder folder;
	std::wstring uri_host(std::wstring host) {
		if (host.starts_with(L"https://")) host = host.substr(8);
		else if (host.starts_with(L"http://")) host = host.substr(7);

		if (host.starts_with(L"www.")) host = host.substr(4);

		if (size_t first_slash = host.find_first_of(L'/')) host = host.substr(0, first_slash);

		return host;
	}
	std::wstring uri_path(std::wstring uri) {
		if (uri.starts_with(L"https://")) uri = uri.substr(8);
		else if (uri.starts_with(L"http://")) uri = uri.substr(7);
		uri = uri.substr(uri.find_first_of(L'/'));

		int query = uri.find_first_of(L'?');
		if(query != -1)uri = uri.substr(0, query);

		return uri;
	}
	bool is_host(std::wstring host, std::wstring match) {
		return host == match || host.ends_with(L"." + match);
	}
	wil::com_ptr<ICoreWebView2Controller> wv_control;
	wil::com_ptr<ICoreWebView2> wv_game;
	std::vector<JSON> wv_game_post;
	std::mutex mtx;
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
	std::wstring cmdline() {
		std::vector<std::wstring> cmds = {
			L"--enable-force-dark",
			// disable-background-timer-throttling
		};

		if (folder.config["client"]["uncap_fps"].get<bool>()) cmds.push_back(L"--disable-frame-rate-limit");

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
	int cmdshow;
	bool fullscreen = false;
	RECT windowed;
	bool enter_fullscreen() {
		if (fullscreen) return false;

		DEVMODE fullscreenSettings;
		
		GetClientRect(&windowed);
		ClientToScreen(&windowed);
		EnumDisplaySettings(NULL, 0, &fullscreenSettings);
		int fullscreenWidth = fullscreenSettings.dmPelsWidth;
		int fullscreenHeight = fullscreenSettings.dmPelsHeight;

		SetWindowLongPtr(GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
		SetWindowLongPtr(GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(0 /* HWND_TOPMOST */, 0, 0, fullscreenWidth, fullscreenHeight, SWP_SHOWWINDOW);
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
	bool check_for_updates() {
		// add https library
		// statically linked, well documented, modern

		std::string send;

		std::string worker = "y9x.github.io";

		for (std::string line : std::vector<std::string>{
			"GET /userscripts/serve.json HTTP/1.0",
			"Host: " + worker,
			"Connection: close",
			"",
		}) send += line + "\r\n";
		
		Socket socket;
		socket.connect(worker, 80);
		socket.send(send);

		std::string body = socket.read();

		socket.close();

		LOG_INFO(body);

		JSON serve = JSON::parse(body);
		
		if (
			VERSION > serve["client"]["version"].get<double>() ||
			::MessageBox(NULL, L"A new client update is available. Download?", title.c_str(), MB_YESNO) != IDYES
		) return false;

		ShellExecute(NULL, L"open", Convert::wstring(serve["client"]["url"].get<std::string>()).c_str(), L"", L"", SW_SHOW);
		PostQuitMessage(EXIT_SUCCESS);
	}
	void init() {
		// if (check_for_updates()) return;

		if (folder.config["window"]["meta"]["replace"].get<bool>())
			title = Convert::wstring(folder.config["window"]["meta"]["title"].get<std::string>());
		
		Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);
		if (folder.config["window"]["meta"]["replace"].get<bool>())
			SetIcon((HICON)LoadImage(hinst, Convert::wstring(folder.config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		else SetIcon(LoadIcon(hinst, MAKEINTRESOURCE(MAINICON)));
		
		NONCLIENTMETRICS metrics = {};
		metrics.cbSize = sizeof(metrics);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
		HFONT default_font = CreateFontIndirect(&metrics.lfCaptionFont);

		CoInitialize(NULL);

		auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

		std::wstring cm = cmdline();
		options->put_AdditionalBrowserArguments(cm.c_str());
		
		CreateCoreWebView2EnvironmentWithOptions(nullptr, (folder.directory + folder.p_profile).c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
			env->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([env, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
				if (controller != nullptr) {
					wv_control = controller;
					wv_control->get_CoreWebView2(&wv_game);
				}

				wil::com_ptr<ICoreWebView2Controller2> controller2;
				controller2 = wv_control.query<ICoreWebView2Controller2>();
				if (controller2) controller2->put_DefaultBackgroundColor(colorref2webview(RGB(255, 255, 255)));

				ICoreWebView2Settings* settings;
				wv_game->get_Settings(&settings);
				settings->put_IsScriptEnabled(TRUE);
				settings->put_AreDefaultScriptDialogsEnabled(TRUE);
				settings->put_IsWebMessageEnabled(TRUE);
#if _DEBUG != 1
				settings->put_AreDevToolsEnabled(FALSE);
#else
				wv_game->OpenDevToolsWindow();
#endif
				settings->put_IsZoomControlEnabled(FALSE);

				resize_wv();

				wv_control->put_ZoomFactor(1);
				
				if (folder.config["client"]["fullscreen"].get<bool>()) enter_fullscreen();
				
				EventRegistrationToken token;

				std::string bootstrap;
				if (load_resource(JS_BOOTSTRAP, bootstrap)) wv_game->AddScriptToExecuteOnDocumentCreated(Convert::wstring(bootstrap).c_str(), nullptr);
				else LOG_ERROR("Error loading bootstrapper");

				// COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SCRIPT
				// recieve all requests for resource swapper
				wv_game->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
				
				wv_game->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([env,this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
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
							// if (load_resource(JS_WEBPACK, js_webpack)) {
							// js_webpack = Manipulate::replace_all(js_webpack, "_RUNTIME_DATA_", data.dump());
							// js_webpack += "//# sourceURL=webpack.js";

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

							if (message[1] == "scripts") open = folder.p_scripts;
							else if (message[1] == "styles") open = folder.p_styles;
							else if (message[1] == "swapper") open = folder.p_swapper;

							ShellExecute(NULL, L"open", (folder.directory + open).c_str(), L"", L"", SW_SHOW);
						}
						else if (event == "relaunch") {
							if (::IsWindow(m_hWnd)) DestroyWindow();

							// https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller?view=webview2-1.0.992.28#close
							wv_control->Close();
							
							init();
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
						else if (event == "browse file") new std::thread([this](JSON id) {
							wchar_t filename[MAX_PATH];

							OPENFILENAME ofn;
							ZeroMemory(&filename, sizeof(filename));
							ZeroMemory(&ofn, sizeof(ofn));
							ofn.lStructSize = sizeof(ofn);
							ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
							ofn.lpstrFilter = L"Icon Files\0*.ico\0Any File\0*.*\0";
							ofn.lpstrFile = filename;
							ofn.nMaxFile = MAX_PATH;
							ofn.lpstrTitle = L"Select a new icon";
							ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

							JSON response = JSON::array();
							response[0] = id;
							// message[1];

							if (GetOpenFileName(&ofn)) {
								response[1] = Convert::string(filename);
								response[2] = false;
							}
							else {
								response[2] = true;
							}
								
							mtx.lock(); 
							wv_game_post.push_back(response);
							mtx.unlock();
						}, message[1]);
					}
					else LOG_ERROR("Recieved invalid message");

					return S_OK;
				}).Get(), &token);

				wv_game->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([env, this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
					// memory doesnt need to be allocated/deallocated since it assigns to a pointer...
					LPWSTR sender_uriptr;
					sender->get_Source(&sender_uriptr);

					ICoreWebView2WebResourceRequest* request;
					args->get_Request(&request);
					LPWSTR uriptr;
					request->get_Uri(&uriptr);
					std::wstring uri = uriptr;
					std::wstring host = uri_host(uri);

					if (is_host(host, L"krunker.io")) {
						std::wstring path = folder.directory + folder.p_swapper + uri_path(uri);
						
						if (IOUtil::file_exists(path)) {
							LOG_INFO("Swapping " << Convert::string(path));
							// Create an empty IStream:
							IStream* stream;
							
							if (SHCreateStreamOnFileEx(path.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, 0, &stream) == S_OK) {
								wil::com_ptr<ICoreWebView2WebResourceResponse> response;
								env->CreateWebResourceResponse(stream, 200, L"OK", L"access-control-allow-origin: https://krunker.io\naccess-control-expose-headers: Content-Length, Content-Type, Date, Server, Transfer-Encoding, X-GUploader-UploadID, X-Google-Trace", &response);
								args->put_Response(response.get());
							}
							else LOG_ERROR("Error creating IStream on path: " << Convert::string(path));
						}
					}else for (std::wstring test : blocked_script_hosts) if (is_host(host, test)) {
						wil::com_ptr<ICoreWebView2WebResourceResponse> response;
						env->CreateWebResourceResponse(nullptr, 403, L"Blocked", L"Content-Type: text/plain", &response);
						args->put_Response(response.get());
						break;
					}

					return S_OK;
				}).Get(), &token);

				wv_game->Navigate(L"https://krunker.io/");

				return S_OK;
			}).Get());

			return S_OK;
		}).Get());

		ResizeClient(700, 500);
		ShowWindow(cmdshow);
		UpdateWindow();

		MSG msg;
		BOOL ret;
		
		LOG_INFO("Client Initialized");
		
		while (ret = GetMessage(&msg, 0, 0, 0)) {
			mtx.lock();
			for (JSON message : wv_game_post) {
				wv_game->PostWebMessageAsJson(Convert::wstring(message.dump()).c_str());
			}
			wv_game_post.clear();
			mtx.unlock();

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	bool resize_wv() {
		if (wv_control == nullptr) return false;

		RECT bounds;
		GetClientRect(&bounds);
		wv_control->put_Bounds(bounds);

		return true;
	}
public:
	Window(HINSTANCE h, int c) : hinst(h), cmdshow(c), folder(L"GC++") {
		init();
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

class LApp : public CAtlExeModuleT<LApp> {
public:
	static HRESULT InitializeCom() {
		CoInitialize(NULL);
		return S_OK;
	}
};

LApp app;

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