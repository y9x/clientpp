#pragma once
#include "../Utils/IOUtil.h"
#include "./ClientFolder.h"
#include "./WebView2Window.h"
#include <mutex>

class KrunkerWindow : public WebView2Window {
public:
	ClientFolder* folder;
	std::vector<JSON> post;
	std::mutex mtx;
	std::wstring og_title;
	std::wstring pathname;
	std::vector<std::wstring> blocked_hosts {
		L"cookie-cdn.cookiepro.com",
		L"googletagmanager.com",
		L"googlesyndication.com",
		L"a.pub.network",
		L"paypalobjects.com",
		L"doubleclick.net",
		L"adinplay.com",
		L"syndication.twitter.com"
	};
	JSON runtime_data();
	std::wstring cmdline();
	void register_events();
	void create(HINSTANCE inst, int cmdshow, std::function<void()> callback) override;
	void get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) override;
	void call_create_webview(std::function<void()> callback);
	void on_dispatch() override;
	KrunkerWindow(ClientFolder& folder, Vector2 scale, std::wstring title, std::wstring path);
};