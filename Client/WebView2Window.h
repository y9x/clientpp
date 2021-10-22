// look into
// https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2?view=webview2-1.0.992.28#addhostobjecttoscript

#pragma once
#include <string>
#include <functional>
#include <atlbase.h>
#include <atlwin.h>
#include <atlenc.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include "./Points.h"

class WebView2Window : public CWindowImpl<WebView2Window> {
public:
	enum class Status {
		Ok,
		MissingRuntime,
		UnknownError,
		AlreadyOpen,
		NotImplemented,
	};
	wil::com_ptr<ICoreWebView2Controller> control;
	wil::com_ptr<ICoreWebView2> webview;
	wil::com_ptr<ICoreWebView2Environment> env;
	std::wstring title;
	Vector2 scale;
	RECT windowed;
	bool open = false;
	bool fullscreen = false;
	HRESULT last_herror = 0;
	HINSTANCE get_hinstance();
	WebView2Window(Vector2 s, std::wstring t);
	~WebView2Window();
	COREWEBVIEW2_COLOR ColorRef(COLORREF color);
	bool create_window(HINSTANCE inst, int cmdshow);
	Status create_webview(std::wstring cmdline, std::wstring directory, std::function<void()> callback);
	bool enter_fullscreen();
	bool exit_fullscreen();
	bool resize_wv();
	bool monitor_data(RECT& rect);
	bool monitor_data(Vector2& pos, Vector2& size);
	virtual Status get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback);
	virtual Status create(HINSTANCE inst, int cmdshow, std::function<void()> callback);
	virtual void on_dispatch();
	LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
	LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
	BEGIN_MSG_MAP(WebView2Window)
		MESSAGE_HANDLER(WM_SIZE, on_resize)
		MESSAGE_HANDLER(WM_DESTROY, on_destroy)
	END_MSG_MAP()
};
