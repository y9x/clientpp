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
	std::mutex mtx;
	std::function<bool(JSMessage)> on_unknown_message;
	std::function<void()> on_webview2_startup;
	std::vector<std::wstring> additional_command_line;
	std::vector<std::wstring> additional_block_hosts;
	std::vector<JSMessage> pending_messages;
	std::wstring og_title;
	RAWINPUTDEVICE rid[1];
	HHOOK mouse_hook = 0;
	bool mouse_hooked = false;
	std::time_t last_pointer_poll;
	nlohmann::json runtime_data();
	void hook_mouse();
	void unhook_mouse();
	void register_events();
	void handle_message(JSMessage msg);
	static LRESULT CALLBACK mouse_message(int code, WPARAM wParam, LPARAM lParam);
	Status call_create_webview(std::function<void()> callback = nullptr);
	std::wstring cmdline();
	std::time_t now();
	std::string status_name(COREWEBVIEW2_WEB_ERROR_STATUS status);
	bool send_resource(ICoreWebView2WebResourceRequestedEventArgs* args, int resource, std::wstring mime);
	void load_userscripts(nlohmann::json* data = nullptr);
	LRESULT on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
public:
	bool can_fullscreen = false;
	COLORREF background = RGB(0, 0, 0);
	ClientFolder* folder;
	std::wstring pathname;
	Status create(HINSTANCE inst, int cmdshow, std::function<void()> callback = nullptr) override;
	Status get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback = nullptr) override;
	void on_dispatch() override;
	bool seek_game();
	KrunkerWindow(ClientFolder& folder, Vector2 scale, std::wstring title, std::wstring path, std::function<void()> webview2_startup = nullptr, std::function<bool(JSMessage)> unknown_message = nullptr);
	~KrunkerWindow();
	BEGIN_MSG_MAP(KrunkerWindow)
		MESSAGE_HANDLER(WM_DESTROY, on_destroy)
		MESSAGE_HANDLER(WM_SIZE, on_resize)
		MESSAGE_HANDLER(WM_INPUT, on_input)
	END_MSG_MAP()
};