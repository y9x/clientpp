#define WILL_LOG 1

#define _CRT_SECURE_NO_WARNINGS

#if WILL_LOG == 1
#include <iostream>
#define LOG(data) std::cout << data << std::endl
#else
#define LOG(data) void()
#endif

#include <windows.h>
#include <vector>
#include <atlbase.h>
#include <atlwin.h>
#include <atlenc.h>
#include <stdlib.h>
#include <string>
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
#include "resource.h"

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
			char* lpc_str = (char*)LockResource(header);
			
			if (lpc_str != NULL) {
				string = lpc_str;
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
private:
	std::wstring name;
	std::vector<std::wstring> directories{ p_scripts, p_styles, p_swapper };
	int last_error = 0;
	// true if the result of CreateDirectory is nonzero or if GetLastError equals ERROR_ALREADY_EXISTS, otherwise false
	bool OVR(int result) {
		if (result != 0)return true;
		else if (GetLastError() == ERROR_ALREADY_EXISTS)return true;
		else return false;
	}
	bool error_creating = false;
public:
	JSON config = JSON::object();
	ClientFolder(std::wstring n) : name(n) {
		directory = _wgetenv(L"USERPROFILE");
		directory += L"\\Documents\\" + name;
		if (OVR(CreateDirectory(directory.c_str(), NULL)))
			for (std::wstring sdir : directories) {
				if (!OVR(CreateDirectory((directory + sdir).c_str(), NULL))) error_creating = true;
			}
		else error_creating = true;

		if (error_creating)LOG("Had an error creating directories");
		else {
			std::string config_buffer;
			
			bool read = false;
			bool needs_save = false;

			IOUtil::wread_file(directory + p_config, config_buffer);

			try {
				config = JSON::parse(config_buffer);
			}
			catch (JSON::exception err) {
				// LOG("caught " << err.what());
				if (load_resource(JSON_CONFIG, config_buffer)) {
					// LOG(config_buffer);
					config = JSON::parse(config_buffer);
					save_config();
				}
			}
		}
	}
	bool save_config() {
		LOG("Wrote config");
		return IOUtil::wwrite_file(directory + p_config, config.dump(1, '\t'));
	}
	std::wstring directory;
	std::wstring p_scripts = L"\\Scripts";
	std::wstring p_styles = L"\\Styles";
	std::wstring p_swapper = L"\\Swapper";
	std::wstring p_config = L"\\Config.json";
};

template<class T>
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
	/*T operator T() {
		return ptr;
	}*/
};

class Window : public CWindowImpl<Window> {
private:
	std::wstring title = L"Guru Client++";
	std::vector<std::wstring> blocked_script_hosts{
		L"cookie-cdn.cookiepro.com",
		L"www.googletagmanager.com",
		L"pagead2.googlesyndication.com",
		L"a.pub.network",
		L"paypalobjects.com"
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
		
		return uri.substr(uri.find_first_of(L'/'));
	}
	bool is_host(std::wstring host, std::wstring match) {
		return host == match || host.ends_with(L'.' + match);
	}
	wil::com_ptr<ICoreWebView2Controller> wv_control;
	wil::com_ptr<ICoreWebView2> wv_window;
	JSON runtime_data() {
		JSON data = JSON::object();

		struct Search {
			std::wstring dir;
			std::wstring filter;
			JSON obj;
		};

		for (Search search : std::vector<Search> {
			{folder.p_styles, L"*.css", data["css"] = JSON::object()},
			{folder.p_scripts, L"*.js", data["js"] = JSON::object()},
		})
			for (IOUtil::WDirectoryIterator it(folder.directory + search.dir, search.filter); ++it;) {
				std::string buffer;

				JSON obj = JSON::object();

				if (IOUtil::read_file(Convert::string(it.path()).c_str(), buffer))
					search.obj[Convert::string(it.file()).c_str()] = buffer;
			}
		
		// std::string js_fixload;
		// if (load_resource(JS_FIXLOAD, js_fixload))injections["js"]["Client/FixLoad.js"] = js_fixload;

		std::string css_client;
		if (load_resource(CSS_CLIENT, css_client)) data["css"]["Client/Client.css"] = css_client;

		// Client/Menu.js
		// if (injections["js"].contains("Menu.js"))injections["js"]["Menu.js"] = Manipulate::replace_all(injections["js"]["Menu.js"].get<std::string>(), "_RUNTIME_CLIENT_CONFIG_", folder.config.dump());

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
public:
	Window(HINSTANCE hInstance, int nCmdShow)  : folder(L"GC++") {
		Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);
		SetIcon(LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)));
		NONCLIENTMETRICS metrics = {};
		metrics.cbSize = sizeof(metrics);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
		HFONT default_font = CreateFontIndirect(&metrics.lfCaptionFont);

		CoInitialize(NULL);
		
		auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
		
		std::wstring commandline;

		if (folder.config["client"]["uncap_fps"].get<bool>()) commandline += L"--disable-frame-rate-limit";

		options->put_AdditionalBrowserArguments(commandline.c_str());
		
		CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
			env->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([env, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
				if (controller != nullptr) {
					wv_control = controller;
					wv_control->get_CoreWebView2(&wv_window);
				}

				wil::com_ptr<ICoreWebView2Controller2> controller2;
				controller2 = wv_control.query<ICoreWebView2Controller2>();
				if (controller2) controller2->put_DefaultBackgroundColor(colorref2webview(RGB(255, 255, 255)));
				
				ICoreWebView2Settings* settings;
				wv_window->get_Settings(&settings);
				settings->put_IsScriptEnabled(TRUE);
				settings->put_AreDefaultScriptDialogsEnabled(TRUE);
				settings->put_IsWebMessageEnabled(TRUE);
#if _DEBUG != 1
				settings->put_AreDevToolsEnabled(FALSE);
#else
				wv_window->OpenDevToolsWindow();
#endif
				settings->put_IsZoomControlEnabled(FALSE);

				resize_wv();

				wv_control->put_ZoomFactor(2);

				EventRegistrationToken token;
				
				std::string bootstrap;
				if (load_resource(JS_BOOTSTRAP, bootstrap)) wv_window->AddScriptToExecuteOnDocumentCreated(Convert::wstring(bootstrap).c_str(), nullptr);
				else LOG("Error loading bootstrapper");

				wv_window->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SCRIPT);
				wv_window->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_STYLESHEET);
					
				wv_window->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
					TaskPtr<PWSTR> mpt;
					args->TryGetWebMessageAsString(&mpt.get());

					if (mpt.valid()) {
						JSON message = JSON::parse(Convert::string(mpt.get()));
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

							sender->PostWebMessageAsString(Convert::wstring(response.dump()).c_str());
						}
						else if (event == "save config") {
							folder.config = message[1];
							folder.save_config();
						}
						else if (event == "open folder") {
							ShellExecute(NULL, L"open", folder.directory.c_str(), L"", L"", SW_SHOW);
						}
					}
					else LOG("Recieved invalid message") << std::endl;
					// processMessage(&message);
					// webview->PostWebMessageAsString(message);

					return S_OK;
				}).Get(), &token);

				wv_window->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([env, this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
					TaskPtr<LPWSTR> sender_uriptr;
					sender->get_Source(&sender_uriptr.get());

					ICoreWebView2WebResourceRequest* request;
					args->get_Request(&request);
					TaskPtr<LPWSTR> uriptr;
					request->get_Uri(&uriptr.get());
					std::wstring uri = uriptr.get();
					std::wstring host = uri_host(uri);

					for (std::wstring test : blocked_script_hosts) {
						if (test == host) {
							wil::com_ptr<ICoreWebView2WebResourceResponse> response;
							env->CreateWebResourceResponse(nullptr, 403, L"Blocked", L"Content-Type: text/plain", &response);
							args->put_Response(response.get());
							break;
						}
					}

					return S_OK;
				}).Get(), &token);

				wv_window->Navigate(L"https://krunker.io/");

				return S_OK;
			}).Get());

			return S_OK;
		}).Get());

		ResizeClient(700, 500);
		ShowWindow(nCmdShow);
		UpdateWindow();

		MSG msg;
		BOOL ret;

		while (ret = GetMessage(&msg, 0, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	~Window() {
		// app shutdown
		// assertion error
		if (::IsWindow(m_hWnd)) DestroyWindow();
	}
	void alert(std::string description, UINT flags = 0) {
		MessageBox(Convert::wstring(description).c_str(), title.c_str(), MB_OK | flags);
	}
	bool prompt(std::string description, UINT flags = 0) {
		return MessageBox(Convert::wstring(description).c_str(), title.c_str(), MB_YESNO | flags) == IDYES;
	}
	bool open_link(std::string url) {
		return ShellExecute(m_hWnd, L"open", Convert::wstring(url).c_str(), NULL, NULL, SW_SHOW);
	}
	void resize_wv() {
		RECT bounds;
		GetClientRect(&bounds);
		wv_control->put_Bounds(bounds);
	}
	LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
		if (wv_control != nullptr) {
			resize_wv();
			return true;
		}

		return false;
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