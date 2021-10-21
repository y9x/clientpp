#pragma once
#include "../Utils/IOUtil.h"
#include "./ClientFolder.h"
#include "./WebView2Window.h"
#include "./IPCMessages.h"
#include <mutex>

class JSMessage {
public:
	IM event = (IM)0;
	nlohmann::json args;
	JSMessage(IM event);
	JSMessage(IM event, nlohmann::json args);
	JSMessage(LPWSTR raw);
	std::string dump();
	bool send(ICoreWebView2* target);
};

class KrunkerWindow : public WebView2Window {
private:
	static LRESULT CALLBACK mouse_message(int code, WPARAM wParam, LPARAM lParam);
	HHOOK mouse_hook = 0;
	bool mouse_hooked = false;
	std::time_t last_pointer_poll;
	nlohmann::json runtime_data();
	void hook_mouse();
	void unhook_mouse();
	void register_events();
	void handle_message(JSMessage msg);
	void call_create_webview(std::function<void()> callback = nullptr);
	std::function<bool(JSMessage)> on_unknown_message;
	std::function<void()> on_webview2_startup;
	std::wstring cmdline();
	std::time_t now();
public:
	bool can_fullscreen = false;
	COLORREF background = RGB(0, 0, 0);
	ClientFolder* folder;
	std::vector<JSMessage> post;
	std::mutex mtx;
	std::wstring og_title;
	std::wstring pathname;
	std::vector<std::wstring> block_hosts;
	void create(HINSTANCE inst, int cmdshow, std::function<void()> callback = nullptr) override;
	void get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback = nullptr) override;
	void on_dispatch() override;
	bool seek_game();
	KrunkerWindow(ClientFolder& folder, Vector2 scale, std::wstring title, std::wstring path, std::function<void()> webview2_startup = nullptr, std::function<bool(JSMessage)> unknown_message = nullptr);
	~KrunkerWindow();
};