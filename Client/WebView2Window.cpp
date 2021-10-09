#include "./WebView2Window.h"
#include "./Log.h"
#include <wrl.h>
#include <WebView2EnvironmentOptions.h>

using Microsoft::WRL::Make;
using Microsoft::WRL::Callback;

HINSTANCE WebView2Window::get_hinstance() {
	return (HINSTANCE)GetWindowLong(GWL_HINSTANCE);
}

bool WebView2Window::create_window(HINSTANCE inst, int cmdshow) {
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

void WebView2Window::create_webview(std::wstring cmdline, std::wstring directory, std::function<void()> callback) {
	auto options = Make<CoreWebView2EnvironmentOptions>();

	options->put_AdditionalBrowserArguments(cmdline.c_str());

	CreateCoreWebView2EnvironmentWithOptions(nullptr, directory.c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this, callback](HRESULT result, ICoreWebView2Environment* envp) -> HRESULT {
		if (envp == nullptr) {
			clog::error << "Env was nullptr" << clog::endl;
			return S_FALSE;
		}
		env = envp;
		env->CreateCoreWebView2Controller(m_hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this, callback](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
			if (controller == nullptr) {
				clog::error << "Controller was nullptr" << clog::endl;
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

COREWEBVIEW2_COLOR WebView2Window::ColorRef(COLORREF color) {
	return COREWEBVIEW2_COLOR{ 255,GetRValue(color),GetGValue(color),GetBValue(color) };
}

bool WebView2Window::enter_fullscreen() {
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
bool WebView2Window::exit_fullscreen() {
	if (!fullscreen) return false;

	SetWindowLongPtr(GWL_EXSTYLE, WS_EX_LEFT);
	SetWindowLongPtr(GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	SetWindowPos(HWND_NOTOPMOST, RECT_ARGS(windowed), SWP_SHOWWINDOW);
	ShowWindow(SW_RESTORE);

	fullscreen = false;

	resize_wv();

	return true;
}
bool WebView2Window::resize_wv() {
	if (control == nullptr) return false;

	RECT bounds;
	GetClientRect(&bounds);
	control->put_Bounds(bounds);

	return true;
}
bool WebView2Window::monitor_data(RECT& rect) {
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
bool WebView2Window::monitor_data(Vector2& pos, Vector2& size) {
	RECT r;

	if (!monitor_data(r))return false;

	pos.x = r.left;
	pos.y = r.top;

	size.x = r.right - r.left;
	size.y = r.bottom - r.top;

	return true;
}
WebView2Window::WebView2Window(Vector2 s, std::wstring t) : title(t), scale(s) {}
WebView2Window::~WebView2Window() {
	// app shutdown
	// assertion error
	if (::IsWindow(m_hWnd)) DestroyWindow();
}
LRESULT WebView2Window::on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	return resize_wv();
}
LRESULT WebView2Window::on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled) {
	open = false;
	return true;
}

void WebView2Window::get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) {}
void WebView2Window::create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {}
void WebView2Window::on_dispatch() {}