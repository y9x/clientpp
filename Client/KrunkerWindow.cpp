#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./KrunkerWindow.h"
#include "../Utils/StringUtil.h"
#include "./LoadRes.h"
#include "./resource.h"
#include "../Utils/Uri.h"
#include "../Utils/Base64.h"
#include "./Log.h"
#include <regex>
#include <WebView2EnvironmentOptions.h>

using namespace StringUtil;
using Microsoft::WRL::Make;
using Microsoft::WRL::Callback;

using JSON = nlohmann::json;

long long KrunkerWindow::now() {
	return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

KrunkerWindow::KrunkerWindow(ClientFolder& folder_, Type type_, Vector2 s, std::wstring title_, std::function<void()> on_startup_, std::function<bool(JSMessage)> on_unknown_message_, std::function<void()> on_destroy_callback_)
	: type(type_)
	, title(title_)
	, og_title(title_)
	, scale(s)
	, folder(&folder_)
	, last_pointer_poll(now())
	, on_webview2_startup(on_startup_)
	, on_unknown_message(on_unknown_message_)
	, on_destroy_callback(on_destroy_callback_)
{
	raw_input.usUsagePage = 0x01;
	raw_input.usUsage = 0x02;
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
	
	return true;
}

COREWEBVIEW2_COLOR KrunkerWindow::ColorRef(COLORREF color) {
	return COREWEBVIEW2_COLOR{ 255,GetRValue(color),GetGValue(color),GetBValue(color) };
}

bool KrunkerWindow::enter_fullscreen() {
	if (fullscreen) return false;

	RECT screen;

	if (!monitor_data(screen)) return false;

	GetClientRect(&saved_size);
	ClientToScreen(&saved_size);

	saved_style = GetWindowLong(GWL_STYLE);
	saved_ex_style = GetWindowLong(GWL_EXSTYLE);
	SetWindowLong(GWL_EXSTYLE, saved_ex_style & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
	SetWindowLong(GWL_STYLE, saved_style & ~(WS_CAPTION | WS_THICKFRAME));
	SetWindowPos(0, &screen, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	resize_wv();

	return fullscreen = true;
}

bool KrunkerWindow::exit_fullscreen() {
	if (!fullscreen) return false;

	SetWindowLong(GWL_STYLE, saved_style);
	SetWindowLong(GWL_EXSTYLE, saved_ex_style);
	SetWindowPos(NULL, RECT_ARGS(saved_size), SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
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

	size.x = r.right - r.left;
	size.y = r.bottom - r.top;

	return true;
}

LRESULT KrunkerWindow::on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	return resize_wv();
}

Vector2 movebuffer;

LRESULT KrunkerWindow::on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	unsigned size = sizeof(RAWINPUT);
	static RAWINPUT raw[sizeof(RAWINPUT)];
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

	if (raw->header.dwType == RIM_TYPEMOUSE) {
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

	return S_OK;
}

LRESULT CALLBACK KrunkerWindow::mouse_message(int code, WPARAM wParam, LPARAM lParam) {
	return 1;
}

void KrunkerWindow::hook_mouse() {
	clog::debug << "Hooking mouse" << clog::endl;
	raw_input.dwFlags = RIDEV_INPUTSINK;
	raw_input.hwndTarget = m_hWnd;

	INPUT input = {};
	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.time = 0;
	input.mi.dwExtraInfo = NULL;
	input.mi.dwFlags = (MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP | MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_MIDDLEUP);
	SendInput(1, &input, sizeof(INPUT));

	RegisterRawInputDevices(&raw_input, 1, sizeof(raw_input));
	mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, *mouse_message, get_hinstance(), NULL);
	mouse_hooked = true;
}

void KrunkerWindow::unhook_mouse() {
	clog::debug << "Unhooked mouse" << clog::endl;
	raw_input.dwFlags = RIDEV_REMOVE;
	raw_input.hwndTarget = NULL;
	RegisterRawInputDevices(&raw_input, 1, sizeof(raw_input));
	UnhookWindowsHookEx(mouse_hook);
	mouse_hooked = false;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring KrunkerWindow::cmdline() {
	load_userscripts();

	std::vector<std::wstring> cmds = {
		L"--disable-background-timer-throttling",
		L"--disable-features=msSmartScreenProtection",
		L"--ignore-gpu-blacklist",
		L"--enable-zero-copy",
		L"--webrtc-max-cpu-consumption-percentage=100",
		L"--autoplay-policy=no-user-gesture-required",
	};

	for (std::wstring cmd : additional_command_line) cmds.push_back(cmd);

	if (folder->config["render"]["uncap_fps"].get<bool>()) {
		cmds.push_back(L"--disable-frame-rate-limit");
		// if (!folder->config["render"]["vsync"])
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

bool KrunkerWindow::send_resource(ICoreWebView2WebResourceRequestedEventArgs* args, int resource, std::wstring mime) {
	IStream* stream = nullptr;
	std::string data;

	if (!load_resource(resource, data)) {
		clog::error << "Unable to load resource " << resource << clog::endl;
		return false;
	}

	if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&stream) != S_OK) {
		clog::error << "Unable to create IStream on HGlobal" << clog::endl;
		return false;
	}
	
	ULONG written = 0;

	if (stream->Write(data.data(), data.size(), &written) != S_OK) {
		clog::error << "Unable to write data to IStream" << clog::endl;
		return false;
	}

	wil::com_ptr<ICoreWebView2WebResourceResponse> response;
	env->CreateWebResourceResponse(stream, 200, L"OK", (L"Content-Type: " + mime).c_str(), &response);
	args->put_Response(response.get());

	return true;
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


	// look into:
	/*ICoreWebView2_2* a;
	ICoreWebView2_3* b;
	ICoreWebView2_4* c;
	ICoreWebView2_5* d;
	ICoreWebView2_6* e;
	*/

	if (wil::com_ptr<ICoreWebView2_2> webview_2 = webview.query<ICoreWebView2_2>()) {
		webview_2->add_ContentLoading(Callback<ICoreWebView2ContentLoadingEventHandler>([this](ICoreWebView2* sender, ICoreWebView2ContentLoadingEventArgs* args) -> HRESULT {
			LPWSTR urip;
			webview->get_Source(&urip);
			Uri uri(urip);

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
	}

	webview->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
		BOOL success = true;
		args->get_IsSuccess(&success);

		LPWSTR urip;
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

KrunkerWindow::Status KrunkerWindow::create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {
	if (!open) {
		if (folder->config["window"]["meta"]["replace"].get<bool>())
			title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());

		create_window(inst, cmdshow);

		SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(background));

		if (can_fullscreen && folder->config["render"]["fullscreen"]) enter_fullscreen();

		if (folder->config["window"]["meta"]["replace"].get<bool>())
			SetIcon((HICON)LoadImage(inst, folder->resolve_path(Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>())).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		else SetIcon(LoadIcon(inst, MAKEINTRESOURCE(MAINICON)));
		
		open = true;
	}

	if (!webview) return call_create_webview(callback);
	else {
		callback();
		return Status::Ok;
	}
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
				if (wil::com_ptr<ICoreWebView2Settings4> settings4 = settings.query<ICoreWebView2Settings4>()) {
					settings4->put_IsGeneralAutofillEnabled(false);
					settings4->put_IsPasswordAutosaveEnabled(false);
				}
			}

			resize_wv();
			register_events();
			
			clog::debug << "KrunkerWindow created" << clog::endl;

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
	if (webview && open) {
		callback(false);
		return Status::AlreadyOpen;
	}
	else {
		return create(inst, cmdshow, [this, callback]() {
			callback(true);
		});
	}
}

long long mouse_hz = 244;
long long mouse_interval = 1000 / mouse_hz;
long long then = KrunkerWindow::now();

void KrunkerWindow::on_dispatch() {
	if (!open) return;
	
	mtx.lock();
	
	for (JSMessage msg : pending_messages)
		if (!msg.send(webview))clog::error << "Unable to send " << msg.dump() << clog::endl;
	
	pending_messages.clear();
	
	for (std::wstring url : pending_navigations) {
		webview->Stop();
		webview->Navigate(url.c_str());
	}
	
	pending_navigations.clear();

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