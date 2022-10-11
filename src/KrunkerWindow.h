#pragma once
#include "../utils/IOUtil.h"
#include "./ClientFolder.h"
#include "./IPCMessages.h"
#include "./JSMessage.h"
#include "./Points.h"
#include <WebView2.h>
#include <atlbase.h>
#include <atlenc.h>
#include <atlwin.h>
#include <functional>
#include <map>
#include <mutex>
#include <rapidjson/fwd.h>
#include <regex>
#include <string>
#include <wil/com.h>
#include <wrl.h>

long long now();

class KrunkerWindow : public CWindowImpl<KrunkerWindow> {
public:
  enum class Status {
    Ok,
    UserDataExists,
    FailCreateUserData,
    RuntimeFatal,
    MissingRuntime,
    UnknownError,
    AlreadyOpen,
    NotImplemented,
  };
  enum class Type {
    Game,
    Social,
    Editor,
    Documents,
    Scripting,
  };

private:
  std::wstring js_frontend;
  std::string css_builtin;
  bool seeking = false;
  std::mutex mtx;
  std::function<bool(JSMessage)> on_unknown_message;
  std::function<void()> on_webview2_startup;
  std::function<void()> on_destroy_callback;
  std::vector<std::wstring> pending_navigations;
  std::vector<JSMessage> pending_messages;
  std::wstring og_title;
  RAWINPUTDEVICE raw_input;
  HHOOK mouse_hook = 0;
  bool mouse_hooked = false;
  std::time_t last_pointer_poll;
  std::string runtime_data();
  void hook_mouse();
  void unhook_mouse();
  void register_events();
  void handle_message(JSMessage msg);
  static LRESULT CALLBACK mouse_message(int code, WPARAM wParam, LPARAM lParam);
  Status call_create_webview(std::function<void()> callback = nullptr);
  std::wstring cmdline();
  bool send_resource(ICoreWebView2WebResourceRequestedEventArgs *args,
                     int resource, std::wstring mime);
  rapidjson::Value
  load_css(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  rapidjson::Value load_userscripts(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  std::map<short, std::pair<std::function<void(const rapidjson::Value &)>,
                            std::function<void(const rapidjson::Value &)>>>
      postedMessages;
  LRESULT on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);

public:
  Type type;
  bool postMessage(const JSMessage &msg,
                   std::function<void(const rapidjson::Value &)> then,
                   std::function<void(const rapidjson::Value &)> catchError);
  bool sendMessage(const JSMessage &message);

  wil::com_ptr<ICoreWebView2Controller> control;
  wil::com_ptr<ICoreWebView2> webview;
  wil::com_ptr<ICoreWebView2Environment> env;
  std::wstring title;
  bool open = false;
  bool fullscreen = false;
  HRESULT last_herror = 0;
  Vector2 scale;
  RECT saved_size;
  DWORD saved_style = 0;
  DWORD saved_ex_style = 0;
  COREWEBVIEW2_COLOR ColorRef(COLORREF color);
  HINSTANCE get_hinstance();
  bool create_window(HINSTANCE inst, int cmdshow);
  bool enter_fullscreen();
  bool exit_fullscreen();
  bool resize_wv();
  bool monitor_data(RECT &rect);
  bool monitor_data(Vector2 &pos, Vector2 &size);
  Status create_webview(std::wstring cmdline, std::wstring directory,
                        std::function<void()> callback);
  bool can_fullscreen = false;
  COLORREF background = RGB(28, 28, 28);
  ClientFolder &folder;
  Status create(HINSTANCE inst, int cmdshow,
                std::function<void()> callback = nullptr);
  Status get(HINSTANCE inst, int cmdshow,
             std::function<void(bool)> callback = nullptr);
  void on_dispatch();
  KrunkerWindow(ClientFolder &folder, Type type, Vector2 scale,
                std::wstring title,
                std::function<void()> webview2_startup = nullptr,
                std::function<bool(JSMessage)> unknown_message = nullptr,
                std::function<void()> on_destroy = nullptr);
  ~KrunkerWindow();
  BEGIN_MSG_MAP(KrunkerWindow)
  MESSAGE_HANDLER(WM_DESTROY, on_destroy)
  MESSAGE_HANDLER(WM_SIZE, on_resize)
  MESSAGE_HANDLER(WM_INPUT, on_input)
  END_MSG_MAP()
};