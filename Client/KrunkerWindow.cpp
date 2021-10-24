#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.hpp>
#include "./KrunkerWindow.h"
#include "../Utils/StringUtil.h"
#include "./LoadRes.h"
#include "./resource.h"
#include "../Utils/Uri.h"
#include "../Utils/Base64.h"
#include "./Log.h"
#include <ShellScalingApi.h>
#include <shellapi.h>
#include <commdlg.h>
#pragma comment(lib, "Shcore.lib")

using namespace StringUtil;
using Microsoft::WRL::Make;
using Microsoft::WRL::Callback;

using JSON = nlohmann::json;

JSMessage::JSMessage(IM e) {
	event = e;
}

JSMessage::JSMessage(IM e, JSON p) {
	event = e;
	args = p;
}

JSMessage::JSMessage(LPWSTR raw) {
	JSON parsed = JSON::parse(Convert::string(raw));

	if (!parsed.is_array()) {
		clog::error << "Message was not an array" << clog::endl;
		return;
	}

	for (size_t index = 0; index < parsed.size(); index++) {
		if (index == 0) {
			if (!parsed[index].is_number()) {
				clog::error << "Event was not a number" << clog::endl;
				return;
			}
			else event = (IM)parsed[index].get<int>();
		}
		else args.push_back(parsed[index]);
	}
}

std::string JSMessage::dump() {
	JSON message = JSON::array();

	message.push_back((int)event);
	for (JSON value : args) message.push_back(value);

	return message.dump();
}

bool JSMessage::send(ICoreWebView2* target) {
	return SUCCEEDED(target->PostWebMessageAsJson(Convert::wstring(dump()).c_str()));
}

KrunkerWindow* awindow;

KrunkerWindow::KrunkerWindow(ClientFolder& f, Vector2 scale, std::wstring title, std::wstring p, std::function<void()> s, std::function<bool(JSMessage)> u)
	: WebView2Window(scale, title)
	, folder(&f)
	, og_title(title)
	, pathname(p)
	, last_pointer_poll(now())
	, on_webview2_startup(s)
	, on_unknown_message(u)
{
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
}

KrunkerWindow::~KrunkerWindow() {
	if (awindow == this) awindow = NULL;
}

bool mousedown = false;

LRESULT CALLBACK KrunkerWindow::mouse_message(int code, WPARAM wParam, LPARAM lParam) {
	MSLLHOOKSTRUCT* hook = (MSLLHOOKSTRUCT*)lParam;
	POINT point = hook->pt;
	
	if (awindow && ::IsWindow(awindow->m_hWnd) && awindow->ScreenToClient(&point)) switch (wParam) {
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN: {

		JSMessage msg(IM::mousedown, { point.x, point.y, wParam == WM_LBUTTONDOWN ? 0 : 2 });
		if (!msg.send(awindow->webview.get()))clog::error << "Unable to send " << msg.dump() << clog::endl;

	} break;
	case WM_LBUTTONUP: 
	case WM_RBUTTONUP: {

		JSMessage msg(IM::mouseup, { point.x, point.y, wParam == WM_LBUTTONUP ? 0 : 2 });
		if (!msg.send(awindow->webview.get()))clog::error << "Unable to send " << msg.dump() << clog::endl;

	} break;
	}
	

	return 1;
	// return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT KrunkerWindow::on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	unsigned size = sizeof(RAWINPUT);
	static RAWINPUT raw[sizeof(RAWINPUT)];
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

	if (raw->header.dwType == RIM_TYPEMOUSE) {
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
			JSMessage msg(IM::mousewheel, { (*(short*)&raw->data.mouse.usButtonData) * -1 /*WHEEL_DELTA*/});
			if (!msg.send(webview.get())) clog::error << "Unable to send " << msg.dump() << clog::endl;
		}
		else {
			POINT cursor;
			GetCursorPos(&cursor);
			ScreenToClient(&cursor);
			JSMessage msg(IM::mousemove, { cursor.x, cursor.y, raw->data.mouse.lLastX, raw->data.mouse.lLastY });
			if (!msg.send(webview.get())) clog::error << "Unable to send " << msg.dump() << clog::endl;
		}
	}

	return 0;
}

void KrunkerWindow::hook_mouse() {
	rid[0].dwFlags = RIDEV_INPUTSINK; 
	rid[0].hwndTarget = m_hWnd;
	RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, *mouse_message, get_hinstance(), NULL);
	mouse_hooked = mouse_hook != NULL;
}

void KrunkerWindow::unhook_mouse() {
	rid[0].dwFlags = RIDEV_REMOVE;
	rid[0].hwndTarget = NULL;
	RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	if (UnhookWindowsHookEx(mouse_hook)) mouse_hooked = false;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring KrunkerWindow::cmdline() {
	load_userscripts();

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

	for (std::wstring cmd : additional_command_line) cmds.push_back(cmd);

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

void KrunkerWindow::handle_message(JSMessage msg) {
	switch ((IM)msg.event) {
	case IM::save_config:
		folder->config = msg.args[0];
		folder->save_config();

		break;
	case IM::open_devtools:
		if (folder->config["client"]["devtools"]) webview->OpenDevToolsWindow();
		break;
	case IM::shell_open: {
		std::wstring open;

		if (msg.args[0] == "root")open = folder->directory;
		else if (msg.args[0] == "logs") open = folder->directory + folder->p_logs;
		else if (msg.args[0] == "scripts") open = folder->directory + folder->p_scripts;
		else if (msg.args[0] == "styles") open = folder->directory + folder->p_styles;
		else if (msg.args[0] == "swapper") open = folder->directory + folder->p_swapper;
		else if (msg.args[0] == "url") open = Convert::wstring(msg.args[1].get<std::string>());

		ShellExecute(m_hWnd, L"open", open.c_str(), L"", L"", SW_SHOW);
	} break;
	case IM::pointer:
		if (msg.args[0] && !mouse_hooked) hook_mouse();
		else if (!msg.args[0] && mouse_hooked) unhook_mouse();

		break;
	case IM::log: {
		std::string log = msg.args[1];

		switch ((LogType)msg.args[0].get<int>()) {
		case LogType::info:
			clog::info << log << clog::endl;
			break;
		case LogType::warn:
			clog::warn << log << clog::endl;
			break;
		case LogType::error:
			clog::error << log << clog::endl;
			break;
		case LogType::debug:
			clog::debug << log << clog::endl;
			break;
		}
	} break;
	case IM::mouse_locked:
		last_pointer_poll = now();
		if (msg.args[0] && !mouse_hooked) hook_mouse();
		else if (!msg.args[0] && mouse_hooked) unhook_mouse();

		break;
	case IM::relaunch_webview:
		// https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller?view=webview2-1.0.992.28#close
		control->Close();
		call_create_webview();

		break;
	case IM::close_window:
		if (::IsWindow(m_hWnd)) DestroyWindow();
		break;
	case IM::reload_window:
		webview->Stop();
		webview->Reload();
		break;
	case IM::seek_game:
		webview->Stop();
		seek_game();
		break;
	case IM::fullscreen:
		if (folder->config["client"]["fullscreen"]) enter_fullscreen();
		else exit_fullscreen();
		break;
	case IM::update_meta:
		title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());
		SetIcon((HICON)LoadImage(get_hinstance(), folder->resolve_path(Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>())).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		SetWindowText(title.c_str());

		break;
	case IM::revert_meta:
		title = og_title;
		SetIcon(LoadIcon(get_hinstance(), MAKEINTRESOURCE(MAINICON)));
		SetWindowText(title.c_str());

		break;
	case IM::reload_config:
		folder->load_config();

		break;
	case IM::browse_file:
		new std::thread([this](JSMessage msg) {
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

			JSMessage res((IM)msg.args[0].get<int>());

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

		break;
	default:
		if (!on_unknown_message || !on_unknown_message(msg)) clog::error << "Unknown message " << msg.dump() << clog::endl;

		break;
	}
}

void KrunkerWindow::register_events() {
	EventRegistrationToken token;

	webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
		LPWSTR mpt;
		args->TryGetWebMessageAsString(&mpt);
		if (!mpt) return S_OK;
		
		handle_message(mpt);

		return S_OK;
	}).Get(), &token);

	webview->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

	/*webview->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
		BOOL success = false;
		args->get_IsSuccess(&success);

		if (!success) {
			// show custom error page?
		}

		return S_OK;
	}).Get(), &token);*/

	webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
		LPWSTR uriptr;
		args->get_Uri(&uriptr);
		Uri uri(uriptr);

		if (uri.host_owns(L"krunker.io")) {
			std::string js_webpack = "throw Error('Failure loading Webpack.js');";
			load_resource(JS_WEBPACK, js_webpack);
			std::string js_webpack_map;
			if (load_resource(JS_WEBPACK_MAP, js_webpack_map)) js_webpack += "\n//# sourceMappingURL=data:application/json;base64," + Base64::Encode(js_webpack_map);

			std::string bootstrap;
			if (load_resource(JS_BOOTSTRAP, bootstrap)) {
				bootstrap = Manipulate::replace_all(bootstrap, "$WEBPACK", JSON(js_webpack).dump());
				bootstrap = Manipulate::replace_all(bootstrap, "$RUNTIME", runtime_data().dump());

				webview->ExecuteScript(Convert::wstring(bootstrap).c_str(), nullptr);
			}
			else clog::error << "Error loading bootstrapper" << clog::endl;
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
		}else for (std::wstring test : additional_block_hosts) if (uri.host_owns(test)) {
			wil::com_ptr<ICoreWebView2WebResourceResponse> response;
			env->CreateWebResourceResponse(nullptr, 403, L"Blocked", L"", &response);
			args->put_Response(response.get());
			break;
		}

		return S_OK;
	}).Get(), &token);
}

HMODULE shcore = LoadLibrary(L"api-ms-win-shcore-scaling-l1-1-1.dll");

KrunkerWindow::Status KrunkerWindow::create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {
	if (folder->config["window"]["meta"]["replace"].get<bool>())
		title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());

	create_window(inst, cmdshow);

	SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(background));
	
	if (can_fullscreen && folder->config["client"]["fullscreen"]) enter_fullscreen();

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

	return call_create_webview(callback);
}

bool KrunkerWindow::seek_game() {
	return SUCCEEDED(webview->Navigate((L"https://krunker.io" + pathname).c_str()));
}

KrunkerWindow::Status KrunkerWindow::call_create_webview(std::function<void()> callback) {
	return create_webview(cmdline(), folder->directory + folder->p_profile, [this, callback]() {
		wil::com_ptr<ICoreWebView2Controller2> control2 = control.query<ICoreWebView2Controller2>();
		if (control2) {
			control2->put_DefaultBackgroundColor(ColorRef(background));
		}
		
		wil::com_ptr<ICoreWebView2Controller3> control3 = control.query<ICoreWebView2Controller3>();
		if (control3) {
			control3->put_ShouldDetectMonitorScaleChanges(false);
		}

		wil::com_ptr<ICoreWebView2Settings> settings;
		ICoreWebView2Settings* se;
		webview->get_Settings(&se);

		if (settings = se) {
			clog::debug << "Settings work" << clog::endl;
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

			wil::com_ptr<ICoreWebView2Settings3> settings3 = settings.query<ICoreWebView2Settings3>();

			if (settings3) {
				settings3->put_AreBrowserAcceleratorKeysEnabled(false);
			}
		}

		resize_wv();

		register_events();

		seek_game();

		clog::debug << "KrunkerWindow created: " << Convert::string(pathname) << clog::endl;

		if (on_webview2_startup) on_webview2_startup();
		if (callback) callback();
	});
}

KrunkerWindow::Status KrunkerWindow::get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) {
	if (!open) return create(inst, cmdshow, [this, callback]() {
		callback(true);
	});
	else {
		callback(false);
		return Status::AlreadyOpen;
	}
}

void KrunkerWindow::on_dispatch() {
	mtx.lock();
	for (JSMessage msg : post)
		if (!msg.send(webview.get()))clog::error << "Unable to send " << msg.dump() << clog::endl;

	post.clear();
	mtx.unlock();

	bool active = GetActiveWindow() == m_hWnd;

	if (!active && mouse_hooked) clog::info << "no fucos" << clog::endl, unhook_mouse();
	else if (!active && IsWindow()) SetWindowText(L"NO FOCUS");
	else if(IsWindow()) SetWindowText(L"FOCUS:");
	if (pathname == L"/" && active) awindow = this;

	if (now() - last_pointer_poll > 1500 && mouse_hooked) {
		clog::error << "Client polling behind" << clog::endl;
		unhook_mouse();
	}
}