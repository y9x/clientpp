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

/*#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"*/

class WebView2Window : public CWindowImpl<WebView2Window> {
public:
	wil::com_ptr<ICoreWebView2Controller> control;
	wil::com_ptr<ICoreWebView2> webview;
	wil::com_ptr<ICoreWebView2Environment> env;
	std::wstring title;
	Vector2 scale;
	RECT windowed;
	bool open = false;
	bool fullscreen = false;
	HINSTANCE get_hinstance();
	WebView2Window(Vector2 s, std::wstring t);
	~WebView2Window();
	COREWEBVIEW2_COLOR ColorRef(COLORREF color);
	bool create_window(HINSTANCE inst, int cmdshow);
	void create_webview(std::wstring cmdline, std::wstring directory, std::function<void()> callback);
	bool enter_fullscreen();
	bool exit_fullscreen();
	bool resize_wv();
	bool monitor_data(RECT& rect);
	bool monitor_data(Vector2& pos, Vector2& size);
	virtual void get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) {}
	virtual void create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {}
	virtual void on_dispatch() {}
	LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
	LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

	BEGIN_MSG_MAP(WebView2Window)
		MESSAGE_HANDLER(WM_SIZE, on_resize)
		MESSAGE_HANDLER(WM_DESTROY, on_destroy)
	END_MSG_MAP()
};
