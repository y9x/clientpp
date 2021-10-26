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
#include <WebView2EnvironmentOptions.h>
#pragma comment(lib, "Shcore.lib")

using namespace StringUtil;
using Microsoft::WRL::Make;
using Microsoft::WRL::Callback;

using JSON = nlohmann::json;

JSMessage::JSMessage() {}

JSMessage::JSMessage(JSON a) {
	args = a;
}

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

bool JSMessage::send(wil::com_ptr<ICoreWebView2> target) {
	return SUCCEEDED(target->PostWebMessageAsJson(Convert::wstring(dump()).c_str()));
}

long long KrunkerWindow::now() {
	return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

KrunkerWindow::KrunkerWindow(ClientFolder& folder_, Type type_, Vector2 s, std::wstring title_, std::wstring pathname_, std::function<void()> on_startup_, std::function<bool(JSMessage)> on_unknown_message_, std::function<void()> on_destroy_callback_)
	: type(type_)
	, title(title_)
	, og_title(title_)
	, scale(s)
	, folder(&folder_)
	, pathname(pathname_)
	, last_pointer_poll(now())
	, on_webview2_startup(on_startup_)
	, on_unknown_message(on_unknown_message_)
	, on_destroy_callback(on_destroy_callback_)
{
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
}

KrunkerWindow::~KrunkerWindow() {
	if (mouse_hooked) unhook_mouse();
	if (::IsWindow(m_hWnd)) DestroyWindow();
}

HINSTANCE KrunkerWindow::get_hinstance() {
	return (HINSTANCE)GetWindowLong(GWL_HINSTANCE);
}

bool KrunkerWindow::create_window(HINSTANCE inst, int cmdshow) {
	Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);

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

COREWEBVIEW2_COLOR KrunkerWindow::ColorRef(COLORREF color) {
	return COREWEBVIEW2_COLOR{ 255,GetRValue(color),GetGValue(color),GetBValue(color) };
}

bool KrunkerWindow::enter_fullscreen() {
	if (fullscreen) return false;

	RECT screen;

	if (!monitor_data(screen)) return false;

	GetClientRect(&windowed);
	ClientToScreen(&windowed);

	DWORD style = GetWindowLong(GWL_STYLE);
	SetWindowLong(GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

	SetWindowPos(0, &screen, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	resize_wv();

	return fullscreen = true;
}

bool KrunkerWindow::exit_fullscreen() {
	if (!fullscreen) return false;

	SetWindowLongPtr(GWL_EXSTYLE, WS_EX_LEFT);
	SetWindowLongPtr(GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

	DWORD style = GetWindowLong(GWL_STYLE);
	SetWindowLong(GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
	SetWindowPos(NULL, RECT_ARGS(windowed), SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	resize_wv();

	fullscreen = false;
	return true;
}

bool KrunkerWindow::resize_wv() {
	if (control == nullptr) return false;

	RECT bounds;
	GetClientRect(&bounds);
	control->put_Bounds(bounds);

	return true;
}

bool KrunkerWindow::monitor_data(RECT& rect) {
	HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(info);

	if (!GetMonitorInfo(monitor, &info)) {
		clog::error << "Can't get monitor info" << clog::endl;
		return false;
	}

	rect = info.rcMonitor;

	return true;
}

bool KrunkerWindow::monitor_data(Vector2& pos, Vector2& size) {
	RECT r;

	if (!monitor_data(r))return false;

	pos.x = r.left;
	pos.y = r.top;

	size.x = (long)r.right - r.left;
	size.y = (long)r.bottom - r.top;

	return true;
}

LRESULT KrunkerWindow::on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	return resize_wv();
}

/*
#define INPUT_IF(id, button) (mouse.usButtonFlags & RI_MOUSE_BUTTON_##id##_DOWN) { \
	msg.event = IM::mousedown; \
	msg.args.push_back(button); \
} else if(mouse.usButtonFlags & RI_MOUSE_BUTTON_##id##_UP) { \
	msg.event = IM::mouseup; \
	msg.args.push_back(button); \
}

*/

Vector2 movebuffer;

LRESULT KrunkerWindow::on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	unsigned size = sizeof(RAWINPUT);
	static RAWINPUT raw[sizeof(RAWINPUT)];
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

	if (raw->header.dwType == RIM_TYPEMOUSE) {
		JSMessage msg;
		RAWMOUSE mouse = raw->data.mouse;
		USHORT flags = mouse.usButtonFlags;

		if (flags & RI_MOUSE_WHEEL) JSMessage(IM::mousewheel, { ((*(short*)&mouse.usButtonData) / WHEEL_DELTA) * -100 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_1_DOWN) JSMessage(IM::mousedown, { 0 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_1_UP) JSMessage(IM::mouseup, { 0 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_2_DOWN) JSMessage(IM::mousedown, { 2 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_2_UP) JSMessage(IM::mouseup, { 2 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_3_DOWN) JSMessage(IM::mousedown, { 1 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_3_UP) JSMessage(IM::mouseup, { 1 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_4_DOWN) JSMessage(IM::mousedown, { 3 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_4_UP) JSMessage(IM::mouseup, { 3 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_5_DOWN) JSMessage(IM::mousedown, { 4 }).send(webview);
		if (flags & RI_MOUSE_BUTTON_5_UP) JSMessage(IM::mouseup, { 4 }).send(webview);
		if (mouse.lLastX || mouse.lLastY) movebuffer += Vector2(raw->data.mouse.lLastX, raw->data.mouse.lLastY); 
	}

	return 0;
}

LRESULT CALLBACK KrunkerWindow::mouse_message(int code, WPARAM wParam, LPARAM lParam) {
	return 1;
}

void KrunkerWindow::hook_mouse() {
	clog::debug << "Hooking mouse" << clog::endl;
	rid[0].dwFlags = RIDEV_INPUTSINK; 
	rid[0].hwndTarget = m_hWnd;
	RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, *mouse_message, get_hinstance(), NULL);
	mouse_hooked = true;
}

void KrunkerWindow::unhook_mouse() {
	clog::debug << "Unhooked mouse" << clog::endl;
	rid[0].dwFlags = RIDEV_REMOVE;
	rid[0].hwndTarget = NULL;
	RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	UnhookWindowsHookEx(mouse_hook);
	mouse_hooked = false;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring KrunkerWindow::cmdline() {
	load_userscripts();

	std::vector<std::wstring> cmds = {
		L"--disable-background-timer-throttling",
		L"--disable-features=msSmartScreenProtection",
		// L"--disable-ipc-flooding-protection",
		L"--force-dark-mode",
		L"--high-dpi-support=1",
		L"--ignore-gpu-blacklist",
		L"--enable-zero-copy",
		L"--webrtc-max-cpu-consumption-percentage=100",
		L"--autoplay-policy=no-user-gesture-required",
	};

	for (std::wstring cmd : additional_command_line) cmds.push_back(cmd);

	if (folder->config["render"]["uncap_fps"].get<bool>()) {
		cmds.push_back(L"--disable-frame-rate-limit");
		/*if (!folder->config["render"]["vsync"])*/
		cmds.push_back(L"--disable-gpu-vsync");
	}
	
	if (folder->config["render"]["angle"] != "default") cmds.push_back(L"--use-angle=" + Convert::wstring(folder->config["render"]["angle"]));
	if (folder->config["render"]["color"] != "default") cmds.push_back(L"--force-color-profile=" + Convert::wstring(folder->config["render"]["color"]));
	
	std::wstring cmdline;
	bool first = false;

	for (std::wstring cmd : cmds) {
		if (first) first = false;
		else cmdline += L" ";

		cmdline += cmd;
	}

	return cmdline;
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
		last_pointer_poll = now();
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
		if (folder->config["game"]["seek"]["F4"]) {
			webview->Stop();
			seek_game();
		}
		break;
	case IM::toggle_fullscreen:
		folder->config["render"]["fullscreen"] = !folder->config["render"]["fullscreen"];
		folder->save_config();
		
		{
			JSMessage msg(IM::update_menu);
			msg.args.push_back(folder->config);
			if (!msg.send(webview))clog::error << "Unable to send " << msg.dump() << clog::endl;
		}
	case IM::fullscreen:
		if (folder->config["render"]["fullscreen"]) enter_fullscreen();
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
			pending_messages.push_back(res);
			mtx.unlock();
		}, msg);

		break;
	default:
		if (!on_unknown_message || !on_unknown_message(msg)) clog::error << "Unknown message " << msg.dump() << clog::endl;

		break;
	}
}

bool KrunkerWindow::send_resource(ICoreWebView2WebResourceRequestedEventArgs* args, int resource, std::wstring mime) {
	// Create an empty IStream:
	IStream* stream = nullptr;
	if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&stream) == S_OK) {
		std::string data;
		if (load_resource(resource, data)) {
			ULONG written = 0;

			if (stream->Write(data.data(), data.size(), &written) == S_OK) {
				wil::com_ptr<ICoreWebView2WebResourceResponse> response;
				env->CreateWebResourceResponse(stream, 200, L"OK", (L"Content-Type: " + mime).c_str(), &response);
				args->put_Response(response.get());

				return true;
			}
			else clog::error << "Unable to write data to IStream" << clog::endl;
		}
		else clog::error << "Unable to load resource " << resource << clog::endl;
	}
	else clog::error << "Unable to create IStream on HGlobal" << clog::endl;

	return false;
}

std::string KrunkerWindow::status_name(COREWEBVIEW2_WEB_ERROR_STATUS status) {
	switch (status) {
	case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_COMMON_NAME_IS_INCORRECT: return "CertificateErrCommonNameIsIncorrect"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_EXPIRED: return "CertificateExpired"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CLIENT_CERTIFICATE_CONTAINS_ERRORS: return "ClientCertificateContainsErrors"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_REVOKED: return "CertificateRevoked"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_IS_INVALID: return "CertificateIsInvalid"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_SERVER_UNREACHABLE: return "ServerUnreachable"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_TIMEOUT: return "Timeout"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_ERROR_HTTP_INVALID_SERVER_RESPONSE: return "ErrorHttpInvalidServerResponse"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_ABORTED: return "ConnectionAborted"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_RESET: return "ConnectionReset"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_DISCONNECTED: return "Disconnected"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_CANNOT_CONNECT: return "CannotConnect"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_HOST_NAME_NOT_RESOLVED: return "HostNameNotResolved"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED: return "OperationCanceled"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_REDIRECT_FAILED: return "RedirectFailed"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_UNEXPECTED_ERROR: return "UnexpectedError"; break;
	case COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN: default: return "Unknown"; break;
	}
}

std::wstring encodeURIComponent(std::wstring decoded){
	std::wostringstream oss;
	std::wregex r(L"[!'\\(\\)*-.0-9A-Za-z_~]");

	for (wchar_t& c : decoded) {
		std::wstring cw;
		cw += c;
		if (std::regex_match(cw, r)) oss << c;
		else oss << "%" << std::uppercase << std::hex << (0xff & c);
	}
	
	return oss.str();
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

	webview->add_NavigationStarting(Callback< ICoreWebView2NavigationStartingEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
		if (mouse_hooked) unhook_mouse();
		return S_OK;
	}).Get(), &token);

	webview->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
		COREWEBVIEW2_PERMISSION_KIND kind;
		args->get_PermissionKind(&kind);
		
		switch (kind) {
		case COREWEBVIEW2_PERMISSION_KIND::COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
		case COREWEBVIEW2_PERMISSION_KIND::COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
			args->put_State(COREWEBVIEW2_PERMISSION_STATE::COREWEBVIEW2_PERMISSION_STATE_ALLOW);
			break;
		}

		return S_OK;
	}).Get(), &token);

	webview->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
		BOOL success = true;
		args->get_IsSuccess(&success);

		wchar_t* urip;
		webview->get_Source(&urip);
		Uri uri(urip);

		if (!success) {
			COREWEBVIEW2_WEB_ERROR_STATUS status;
			args->get_WebErrorStatus(&status);

			if (status == COREWEBVIEW2_WEB_ERROR_STATUS::COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_ABORTED)return S_OK;

			// renderer freezes when navigating from an error page that occurs on startup
			control->Close();
			call_create_webview([this, status, uri]() {
				JSON data = { status };
				data.push_back(status_name(status));
				data.push_back(Convert::string(uri.href));
				webview->Navigate((L"https://chief/error?data=" + encodeURIComponent(Convert::wstring(data.dump()))).c_str());
			});
		}else if (uri.host_owns(L"krunker.io")) {
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
		std::wstring pathname = uri.pathname();

		if (pathname == L"/favicon.ico") {
			wil::com_ptr<ICoreWebView2WebResourceResponse> response;
			env->CreateWebResourceResponse(nullptr, 403, L"Unused", L"", &response);
			args->put_Response(response.get());
		}else if (uri.host() == L"chief") {
			if (pathname == L"/error") send_resource(args, HTML_ERROR, L"text/html");
			else if (pathname == L"/GameFont.ttf") send_resource(args, FONT_GAME, L"font/ttf");
		}else if (uri.host_owns(L"krunker.io")) {
			std::wstring swap = folder->directory + folder->p_swapper + pathname;

			if (IOUtil::file_exists(swap)) {
				clog::info << "Swapping " << Convert::string(pathname) << clog::endl;
				// Create an empty IStream:
				IStream* stream;

				if (SHCreateStreamOnFileEx(swap.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, 0, &stream) == S_OK) {
					wil::com_ptr<ICoreWebView2WebResourceResponse> response;
					env->CreateWebResourceResponse(stream, 200, L"OK", L"access-control-allow-origin: https://krunker.io\naccess-control-expose-headers: Content-Length, Content-Type, Date, Server, Transfer-Encoding, X-GUploader-UploadID, X-Google-Trace", &response);
					args->put_Response(response.get());
				}
				else clog::error << "Unable to create IStream for file: " << Convert::string(swap) << clog::endl;
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
	
	if (can_fullscreen && folder->config["render"]["fullscreen"]) enter_fullscreen();

	if (shcore) {
		using dec = decltype(::SetProcessDpiAwareness);
		std::function SetProcessDpiAwareness = (dec*)GetProcAddress(shcore, "SetProcessDpiAwareness");

		if (SetProcessDpiAwareness) {
			HRESULT sda = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			if (!SUCCEEDED(sda)) clog::error << "SetProcessDpiAwareness returned " << std::hex << HRESULT_CODE(sda) << clog::endl;
		}
		else clog::error << "Unable to get address of SetProcessDpiAwareness" << clog::endl;
	}else clog::warn << "Unable to load shcore. Is the host Win7?" << clog::endl;
	
	if (folder->config["window"]["meta"]["replace"].get<bool>())
		SetIcon((HICON)LoadImage(inst, folder->resolve_path(Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>())).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
	else SetIcon(LoadIcon(inst, MAKEINTRESOURCE(MAINICON)));

	return call_create_webview(callback);
}

KrunkerWindow::Status KrunkerWindow::call_create_webview(std::function<void()> callback) {
	auto options = Make<CoreWebView2EnvironmentOptions>();

	options->put_AdditionalBrowserArguments(cmdline().c_str());

	HRESULT create = CreateCoreWebView2EnvironmentWithOptions(nullptr, (folder->directory + folder->p_profile).c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this, callback](HRESULT result, ICoreWebView2Environment* envp) -> HRESULT {
		if (envp == nullptr) {
			clog::error << "Env was nullptr" << clog::endl;

			return S_FALSE;
		}

		env = envp;
		env->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this, callback](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
			if (controller == nullptr) {
				clog::error << "Controller was nullptr. Error code: 0x" << std::hex << result  << clog::endl;
				// call_create_webview(callback);
				return S_FALSE;
			}

			control = controller;
			control->get_CoreWebView2(&webview);

			if (wil::com_ptr<ICoreWebView2Controller2> control2 = control.query<ICoreWebView2Controller2>()) {
				control2->put_DefaultBackgroundColor(ColorRef(background));
			}

			if (wil::com_ptr<ICoreWebView2Controller3> control3 = control.query<ICoreWebView2Controller3>()) {
				control3->put_ShouldDetectMonitorScaleChanges(false);
			}

			wil::com_ptr<ICoreWebView2Settings> settings;
			if (SUCCEEDED(webview->get_Settings(&settings))) {
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

				if (wil::com_ptr<ICoreWebView2Settings3> settings3 = settings.query<ICoreWebView2Settings3>()) {
					settings3->put_AreBrowserAcceleratorKeysEnabled(false);
				}
			}

			resize_wv();
			register_events();
			seek_game();

			clog::debug << "KrunkerWindow created: " << Convert::string(pathname) << clog::endl;

			if (on_webview2_startup) on_webview2_startup();
			if (callback) callback();

			return S_OK;
		}).Get());

		return S_OK;
	}).Get());

	if (!SUCCEEDED(create)) {
		last_herror = create;

		switch (create) {
		case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
			return Status::MissingRuntime;
			break;
		case HRESULT_FROM_WIN32(ERROR_FILE_EXISTS):
			return Status::UserDataExists;
			break;
		case E_ACCESSDENIED:
			return Status::FailCreateUserData;
			break;
		case E_FAIL:
			return Status::RuntimeFatal;
			break;
		default:
			return Status::UnknownError;
			break;
		}
	}
	else return Status::Ok;
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

long long mouse_hz = 240;
long long mouse_interval = 1000 / mouse_hz;
long long then = KrunkerWindow::now();

void KrunkerWindow::on_dispatch() {
	if (!open) return;
	
	mtx.lock();
	
	for (JSMessage msg : pending_messages)
		if (!msg.send(webview))clog::error << "Unable to send " << msg.dump() << clog::endl;
	pending_messages.clear();
	
	mtx.unlock();

	bool active = GetActiveWindow() == m_hWnd;

	if (!active && mouse_hooked) unhook_mouse();
	
	time_t last_poll = now() - last_pointer_poll;

	if (last_poll > 1500 && mouse_hooked) {
		clog::error << "Pointer lock timeout: " << last_poll << "ms" << clog::endl;
		unhook_mouse();
	}

	if (mouse_hooked) {
		long long nw = now();
		long long delta = nw - then;

		if (delta > mouse_interval) {
			then = nw - (delta % mouse_interval);
			JSMessage(IM::mousemove, { movebuffer.x, movebuffer.y }).send(webview);
			movebuffer.clear();
		}

	}
}

LRESULT KrunkerWindow::on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	open = false;
	if (on_destroy_callback) on_destroy_callback();
	return true;
}