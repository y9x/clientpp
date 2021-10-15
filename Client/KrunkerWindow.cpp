#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./httplib.h"
#include "./KrunkerWindow.h"
#include "../Utils/StringUtil.h"
#include "./LoadRes.h"
#include "./resource.h"
#include "../Utils/Uri.h"
#include "../Utils/Base64.h"
#include "./Log.h"
#include <ShellScalingApi.h>

#pragma comment(lib, "Shcore.lib")

using namespace StringUtil;
using Microsoft::WRL::Make;
using Microsoft::WRL::Callback;

JSMessage::JSMessage(std::string e) : event(e) {}
JSMessage::JSMessage(std::string e, JSON p) : event(e), args(p) {}
JSMessage::JSMessage(std::wstring raw) {
	JSON message = JSON::parse(Convert::string(raw));

	for (size_t index = 0; index < message.size(); index++)
		if (index == 0)event = message[index];
		else args.push_back(message[index]);
}

JSON JSMessage::json() {
	JSON data = JSON::array();

	data[0] = event.c_str();

	for (JSON value : args)data.push_back(value);

	return data;
}

bool JSMessage::send(ICoreWebView2* target) {
	return SUCCEEDED(target->PostWebMessageAsJson(Convert::wstring(json().dump()).c_str()));
}

KrunkerWindow::KrunkerWindow(ClientFolder& f, Vector2 scale, std::wstring title, std::wstring p, std::function<void()> s)
	: WebView2Window(scale, title)
	, folder(&f)
	, og_title(title)
	, pathname(p)
	, webview2_startup(s)
	, last_client_poll(now())
{}

JSON KrunkerWindow::runtime_data() {
	JSON data = JSON::object();

	struct Search {
		std::wstring dir;
		std::wstring filter;
		JSON& obj;
	};

	for (Search search : std::vector<Search>{
		{folder->p_styles, L"*.css", data["css"] = JSON::object()},
		{folder->p_scripts, L"*.js", data["js"] = JSON::object()},
	})
		for (IOUtil::WDirectoryIterator it(folder->directory + search.dir, search.filter); ++it;) {
			std::string buffer;

			if (IOUtil::read_file(it.path().c_str(), buffer))
				search.obj[Convert::string(it.file()).c_str()] = buffer;
		}

	std::string css_client;
	if (load_resource(CSS_CLIENT, css_client)) data["css"]["Client/Client.css"] = css_client;

	data["config"] = folder->config;

	return data;
}

KrunkerWindow* active_window;

bool mousedown = false;

LRESULT CALLBACK KrunkerWindow::mouse_message(int code, WPARAM wParam, LPARAM lParam) {
	if (wParam == WM_LBUTTONDOWN) {
		MSLLHOOKSTRUCT* hook = (MSLLHOOKSTRUCT*)lParam;
		POINT point = hook->pt;

		if (active_window->ScreenToClient(&point)) {
			JSMessage msg("mousedown", { point.x, point.y });
			if (!msg.send(active_window->webview.get()))clog::error << "Unable to send " << msg.json() << clog::endl;
		}

		return 1;
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

void KrunkerWindow::hook_mouse() {
	mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, *mouse_message, get_hinstance(), NULL);
	mouse_hooked = mouse_hook != NULL;
}

void KrunkerWindow::unhook_mouse() {
	if (UnhookWindowsHookEx(mouse_hook)) mouse_hooked = false;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring KrunkerWindow::cmdline() {
	std::vector<std::wstring> cmds = {
		// on chrome 86
		// L"--enable-features=WebAssembly,SharedArrayBuffer",
		// L"--js-flags=--experimental-wasm-threads",
		// bad for cpu
		L"--disable-background-timer-throttling",
		L"--disable-features=msSmartScreenProtection",
		L"--force-dark-mode",
		L"--high-dpi-support=1",
		L"--ignore-gpu-blacklist",
		L"--disable-print-preview",
		L"--enable-zero-copy",
		L"--webrtc-max-cpu-consumption-percentage=100",
		L"--autoplay-policy=no-user-gesture-required",
		L"--disable-ipc-flooding-protection",
		// L"--profile-directory=Profile",
	};

	if (folder->config["client"]["uncap_fps"].get<bool>()) {
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

std::time_t KrunkerWindow::now() {
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void KrunkerWindow::register_events() {
	EventRegistrationToken token;

	std::string bootstrap;
	if (load_resource(JS_BOOTSTRAP, bootstrap)) webview->AddScriptToExecuteOnDocumentCreated(Convert::wstring(bootstrap).c_str(), nullptr);
	else clog::error << "Error loading bootstrapper" << clog::endl;

	webview->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
	webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
		LPWSTR mpt;
		args->TryGetWebMessageAsString(&mpt);

		if (mpt) {
			JSMessage msg(mpt);
			JSON message = JSON::parse(Convert::string(mpt));

			if (msg.event == "send webpack") {
				std::string js_webpack = "throw Error('Failure loading Webpack.js');";
				load_resource(JS_WEBPACK, js_webpack);
				std::string js_webpack_map;
				if (load_resource(JS_WEBPACK_MAP, js_webpack_map)) js_webpack += "\n//# sourceMappingURL=data:application/json;base64," + Base64::Encode(js_webpack_map);

				JSMessage res("eval webpack", { js_webpack, runtime_data() });
				if (!res.send(sender))clog::error << "Unable to send " << res.json() << clog::endl;
			}
			else if (msg.event == "log") {
				std::string log = msg.args[1];

				if (msg.args[0] == "info")clog::info << log << clog::endl;
				else if (msg.args[0] == "warn")clog::warn << log << clog::endl;
				else if (msg.args[0] == "error")clog::error << log << clog::endl;
				else if (msg.args[0] == "debug")clog::debug << log << clog::endl;
			}
			else if (msg.event == "open devtools") {
				if (folder->config["client"]["devtools"]) webview->OpenDevToolsWindow();
			}
			else if (msg.event == "save config") {
				folder->config = msg.args[0];
				folder->save_config();
			}
			else if (msg.event == "open") {
				std::wstring open;

				if (msg.args[0] == "root")open = folder->directory;
				else if (msg.args[0] == "logs") open = folder->directory + folder->p_logs;
				else if (msg.args[0] == "scripts") open = folder->directory + folder->p_scripts;
				else if (msg.args[0] == "styles") open = folder->directory + folder->p_styles;
				else if (msg.args[0] == "swapper") open = folder->directory + folder->p_swapper;
				else if (msg.args[0] == "url") open = Convert::wstring(msg.args[1].get<std::string>());

				ShellExecute(m_hWnd, L"open", open.c_str(), L"", L"", SW_SHOW);
			}
			else if (msg.event == "pointer") {
				if (msg.args[0] == "hook" && !mouse_hooked) hook_mouse();
				else if (mouse_hooked) unhook_mouse();
			}
			else if (msg.event == "mouse locked") {
				last_client_poll = now();
				if (msg.args[0] && !mouse_hooked) hook_mouse();
				else if (!msg.args[0] && mouse_hooked) unhook_mouse();
			}
			else if (msg.event == "relaunch") {
				// https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller?view=webview2-1.0.992.28#close
				control->Close();

				call_create_webview([]() {});
			}
			else if (msg.event == "close window") {
				if (::IsWindow(m_hWnd)) DestroyWindow();
			}
			else if (msg.event == "seek game") {
				webview->Stop();
				seek_game();
			}
			else if (msg.event == "reload") {
				webview->Stop();
				webview->Reload();
			}
			else if (msg.event == "fullscreen") {
				if (folder->config["client"]["fullscreen"]) enter_fullscreen();
				else exit_fullscreen();
			}
			else if (msg.event == "update meta") {
				title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());
				SetIcon((HICON)LoadImage(get_hinstance(), folder->resolve_path(Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>())).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
				SetWindowText(title.c_str());
			}
			else if (msg.event == "revert meta") {
				title = og_title;
				SetIcon(LoadIcon(get_hinstance(), MAKEINTRESOURCE(MAINICON)));
				SetWindowText(title.c_str());
			}
			else if (msg.event == "reload config") {
				folder->load_config();
			}
			else if (msg.event == "browse file") new std::thread([this](JSMessage msg) {
				wchar_t filename[MAX_PATH];

				OPENFILENAME ofn;
				ZeroMemory(&filename, sizeof(filename));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_hWnd;
				std::wstring title = Convert::wstring(msg.args[1]);
				std::wstring filters;
				for (JSON value : msg.args[2]) {
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

				JSMessage res(msg.args[0].get<std::string>());

				if (GetOpenFileName(&ofn)) {
					std::wstring fn;
					fn.resize(MAX_PATH);
					fn = filename;

					// make relative
					res.args[0] = Convert::string(folder->relative_path(fn));
					res.args[1] = false;
				}
				else res.args[1] = true;
				
				mtx.lock();
				post.push_back(res);
				mtx.unlock();
			}, msg);
			else clog::error << "Unknown message " << msg.json() << clog::endl;
		}

		return S_OK;
	}).Get(), &token);

	webview->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
		LPWSTR sender_uriptr;
		sender->get_Source(&sender_uriptr);

		ICoreWebView2WebResourceRequest* request = 0;
		args->get_Request(&request);
		LPWSTR uriptr;
		request->get_Uri(&uriptr);
		Uri uri(uriptr);
		request->Release();

		if (uri.host_owns(L"krunker.io")) {
			if (uri.pathname().starts_with(L"/pkg/")) {
				IStream* stream = nullptr;
				// request resource manually because webview2 fails to cache
				if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&stream) == S_OK) {
					httplib::Client cli(Convert::string(uri.origin()));
					auto res = cli.Get(Convert::string(uri.pathname()).c_str());

					ULONG written = 0;

					if (stream->Write(res->body.data(), res->body.length(), &written) == S_OK) {
						wil::com_ptr<ICoreWebView2WebResourceResponse> response;
						env->CreateWebResourceResponse(stream, 200, L"Swapped", L"", &response);
						args->put_Response(response.get());
					}
					else clog::error << "Error writing to IStream" << clog::endl;

				}
				else clog::error << "Error creating IStream on HGlobal" << clog::endl;
			}
			else {
				std::wstring swap = folder->directory + folder->p_swapper + uri.pathname();

				if (IOUtil::file_exists(swap)) {
					clog::info << "Swapping " << Convert::string(uri.pathname()) << clog::endl;
					// Create an empty IStream:
					IStream* stream;

					if (SHCreateStreamOnFileEx(swap.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, 0, &stream) == S_OK) {
						wil::com_ptr<ICoreWebView2WebResourceResponse> response;
						env->CreateWebResourceResponse(stream, 200, L"OK", L"access-control-allow-origin: https://krunker.io\naccess-control-expose-headers: Content-Length, Content-Type, Date, Server, Transfer-Encoding, X-GUploader-UploadID, X-Google-Trace", &response);
						args->put_Response(response.get());
					}
					else clog::error << "Error creating IStream on swap: " << Convert::string(swap) << clog::endl;
				}
			}
		}
		else if (folder->config["client"]["adblock"].get<bool>()) for (std::wstring test : ad_hosts) if (uri.host_owns(test)) {
			wil::com_ptr<ICoreWebView2WebResourceResponse> response;
			env->CreateWebResourceResponse(nullptr, 403, L"Ad", L"", &response);
			args->put_Response(response.get());
			break;
		}

		return S_OK;
	}).Get(), &token);
}

std::map<std::string, void*> loaded;

void* library_functionv(std::string function, std::string dll) {
	std::string prop = dll + function;

	if (!loaded.contains(prop)) {
		HMODULE lib = LoadLibrary(Convert::wstring(dll).c_str());

		if (lib)loaded[prop] = (void*)GetProcAddress(lib, function.c_str());
	}

	return loaded[prop];
}

HMODULE shcore = LoadLibrary(L"api-ms-win-shcore-scaling-l1-1-1.dll");

template<class T>
inline T* LibraryFunction(std::string function, std::string dll) {
	return (T*)library_functionv(function, dll);
}

void KrunkerWindow::create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {
	if (folder->config["window"]["meta"]["replace"].get<bool>())
		title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());

	create_window(inst, cmdshow);
	
	SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(0, 0, 0)));

	if (folder->config["client"]["fullscreen"]) enter_fullscreen();

	if (shcore) {
		using dec = decltype(::SetProcessDpiAwareness);
		std::function SetProcessDpiAwareness = (dec*)GetProcAddress(shcore, "SetProcessDpiAwareness");

		if (SetProcessDpiAwareness) {
			HRESULT sda = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			if (!SUCCEEDED(sda)) clog::error << "SetProcessDpiAwareness returned " << PROCESS_PER_MONITOR_DPI_AWARE << clog::endl;
		}
		else clog::error << "Unable to get address of SetProcessDpiAwareness" << clog::endl;
	}else clog::warn << "Unable to load shcore, is the host Win7?" << clog::endl;
	
	if (folder->config["window"]["meta"]["replace"].get<bool>())
		SetIcon((HICON)LoadImage(inst, folder->resolve_path(Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>())).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
	else SetIcon(LoadIcon(inst, MAKEINTRESOURCE(MAINICON)));

	call_create_webview(callback);
}

bool KrunkerWindow::seek_game() {
	return SUCCEEDED(webview->Navigate((L"https://krunker.io" + pathname).c_str()));
}

void KrunkerWindow::call_create_webview(std::function<void()> callback) {
	create_webview(cmdline(), folder->directory + folder->p_profile, [this, callback]() {
		wil::com_ptr<ICoreWebView2Controller2> control2;
		control2 = control.query<ICoreWebView2Controller2>();
		if (control2) {
			control2->put_DefaultBackgroundColor(ColorRef(RGB(0, 0, 0)));
		}
		
		wil::com_ptr<ICoreWebView2Controller3> control3;
		control3 = control.query<ICoreWebView2Controller3>();
		if (control3) {
			control3->put_ShouldDetectMonitorScaleChanges(true);
		}

		wil::com_ptr<ICoreWebView2Settings> settings;
		ICoreWebView2Settings* se;
		webview->get_Settings(&se);

		if (settings = se) {
			clog::info << "Settings work" << clog::endl;
			settings->put_IsScriptEnabled(true);
			settings->put_AreDefaultScriptDialogsEnabled(true);
			settings->put_IsWebMessageEnabled(true);
			settings->put_IsZoomControlEnabled(false);
			settings->put_AreDefaultContextMenusEnabled(false);
			settings->put_IsStatusBarEnabled(false);
#if _DEBUG != 1
			settings->put_AreDevToolsEnabled(false);
#else
			webview->OpenDevToolsWindow();
#endif

			wil::com_ptr<ICoreWebView2Settings3> settings3;

			if (settings3 = settings.query<ICoreWebView2Settings3>()) {
				settings3->put_AreBrowserAcceleratorKeysEnabled(false);
			}
		}

		resize_wv();

		register_events();

		seek_game();

		clog::debug << "KrunkerWindow created: " << Convert::string(pathname) << clog::endl;

		if (webview2_startup) webview2_startup();
		if (callback) callback();
	});
}

void KrunkerWindow::get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) {
	if (!open) create(inst, cmdshow, [this, callback]() {
		callback(true);
	});
	else callback(false);
}

void KrunkerWindow::on_dispatch() {
	mtx.lock();
	for (JSMessage msg : post)
		if (!msg.send(webview.get()))clog::error << "Unable to send " << msg.json() << clog::endl;

	post.clear();
	mtx.unlock();

	if (pathname == L"/" && GetActiveWindow() == m_hWnd) active_window = this;

	if (now() - last_client_poll > 2000 && mouse_hooked) {
		clog::error << "Client polling behind" << clog::endl;
		unhook_mouse();
	}
}
