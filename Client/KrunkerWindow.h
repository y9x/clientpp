#pragma once
#include "../Utils/IOUtil.h"
#include "./ClientFolder.h"
#include "./WebView2Window.h"
#include <mutex>

class JSMessage {
public:
	std::string event;
	JSON args;
	JSMessage(std::string e);
	JSMessage(std::string e, JSON p);
	JSMessage(std::wstring raw);
	JSON json();
	bool send(ICoreWebView2* target);
};

class KrunkerWindow : public WebView2Window {
private:
	static LRESULT CALLBACK mouse_message(int code, WPARAM wParam, LPARAM lParam);
	HHOOK mouse_hook = 0;
	bool mouse_hooked = false;
	std::time_t last_client_poll;
	void hook_mouse();
	void unhook_mouse();
	JSON runtime_data();
	void register_events();
	void call_create_webview(std::function<void()> callback);
	std::function<void(JSMessage)> on_unknown_message;
	std::function<void()> on_webview2_startup;
	std::wstring cmdline();
	std::time_t now();
public:
	ClientFolder* folder;
	std::vector<JSMessage> post;
	std::mutex mtx;
	std::wstring og_title;
	std::wstring pathname;
	std::vector<std::wstring> ad_hosts {
		L"cookie-cdn.cookiepro.com",
		L"googletagmanager.com",
		L"googlesyndication.com",
		L"pub.network",
		L"paypalobjects.com",
		L"doubleclick.net",
		L"adinplay.com",
		L"syndication.twitter.com"
	};
	void create(HINSTANCE inst, int cmdshow, std::function<void()> callback = nullptr) override;
	void get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback = nullptr) override;
	void on_dispatch() override;
	bool seek_game();
	KrunkerWindow(ClientFolder& folder, Vector2 scale, std::wstring title, std::wstring path, std::function<void()> webview2_startup = nullptr, std::function<void(JSMessage)> unknown_message = nullptr);
	~KrunkerWindow();
};