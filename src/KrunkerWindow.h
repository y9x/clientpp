#pragma once
#include "../utils/IOUtil.h"
#include "./AccountManager.h"
#include "./ClientFolder.h"
#include "./IPCMessages.h"
#include "./JSMessage.h"
#include "./Points.h"
#include "./Site.h"
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

class ChWindows;

class ChWindow : public CWindowImpl<ChWindow> {
private:
  std::wstring cmdLine();
  LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);

protected:
  // Every window should be able to access other windows
  ChWindows &windows;
  ClientFolder &folder;
  std::vector<std::wstring> pendingNavigations;
  // todo: never overwrite og_title & title
  std::wstring title;
  std::wstring og_title;
  std::mutex dispatchMtx;
  virtual void registerEvents();
  Status callCreateWebView(std::function<void()> callback = nullptr);
  wil::com_ptr<ICoreWebView2Controller> control;
  wil::com_ptr<ICoreWebView2> webview;
  wil::com_ptr<ICoreWebView2Environment> env;
  bool open = false;
  HRESULT last_herror = 0;
  Vector2 scale;
  HINSTANCE getHinstance();
  bool createWindow();
  bool resizeWV();
  bool monitorData(RECT &rect);
  bool monitorData(Vector2 &pos, Vector2 &size);
  Status createWebView(std::wstring cmdline, std::wstring directory,
                       std::function<void()> callback);
  virtual Status create(std::function<void()> callback = nullptr);
  Status get(std::function<void(bool)> callback = nullptr);
  BEGIN_MSG_MAP(ChWindow)
  MESSAGE_HANDLER(WM_DESTROY, on_destroy)
  MESSAGE_HANDLER(WM_SIZE, on_resize)
  END_MSG_MAP()
public:
  virtual void dispatch();
  Status show(UriW uri, ICoreWebView2 *sender = nullptr,
              std::function<void()> open = nullptr,
              bool &shown = *(bool *)nullptr);
  ChWindow(ClientFolder &folder, ChWindows &windows, Vector2 scale,
           std::wstring title);
  ~ChWindow();
};

class ChScriptedWindow : public ChWindow {
private:
  // Window-Frontend messaging:

  AccountManager &accounts;
  std::map<short, std::pair<std::function<void(const rapidjson::Value &)>,
                            std::function<void(const rapidjson::Value &)>>>
      postedMessages;
  // Mouse hooking:
  static LRESULT CALLBACK mouseMessage(int code, WPARAM wParam, LPARAM lParam);
  LRESULT on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  void hook_mouse();
  void unhook_mouse();
  RAWINPUTDEVICE raw_input;
  HHOOK mouse_hook = 0;
  bool mouse_hooked = false;
  std::time_t last_pointer_poll;

  long long mouse_hz = 244;
  long long mouse_interval = 1000 / mouse_hz;
  long long then = now();

  // Fullscreen:
  RECT saved_size;
  DWORD saved_style = 0;
  DWORD saved_ex_style = 0;
  bool fullscreen = false;
  bool enterFullscreen();
  bool exitFullscreen();
  std::vector<JSMessage> pendingMessages;
  std::wstring mainJS;
  std::string mainCSS;
  bool seeking = false;
  bool sendResource(ICoreWebView2WebResourceRequestedEventArgs *args,
                    int resource, std::wstring mime);
  bool postMessage(const JSMessage &msg,
                   std::function<void(const rapidjson::Value &)> then,
                   std::function<void(const rapidjson::Value &)> catchError);
  bool sendMessage(const JSMessage &message);
  std::string runtimeData();
  void handleMessage(JSMessage msg);
  rapidjson::Value
  loadCSS(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  rapidjson::Value loadUserScripts(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  void registerEvents() override;
  Status create(std::function<void()> callback = nullptr) override;
  // no longer allowing client to intercept messages...
  // std::function<bool(JSMessage)> on_unknown_message;
  BEGIN_MSG_MAP(ChScriptedWindow)
  CHAIN_MSG_MAP(ChWindow)
  MESSAGE_HANDLER(WM_INPUT, on_input)
  END_MSG_MAP()
public:
  ChScriptedWindow(ClientFolder &folder, AccountManager &accounts,
                   ChWindows &windows, Vector2 scale, std::wstring title);
  ~ChScriptedWindow();
  void dispatch() override;
};

class ChWindows {
private:
  ChScriptedWindow game;
  ChScriptedWindow social;
  ChWindow editor;
  ChWindow viewer;
  ChWindow scripting;

public:
  void dispatch();
  ChWindows(ClientFolder &folder, AccountManager &accounts);
  /// @brief
  /// @param type Type of window.
  /// @return The pointer to the window member. May be nullptr if unknown was
  /// specified.
  ChWindow *getWindow(krunker::type type);
  /// @brief
  /// @param uri Window to reference to determine if navigation should be
  /// blocked.
  /// @param sender Called when the window is created/fetched. newWindow will
  /// be a nullptr if the URL was opened via the shell
  /// @param open
  /// @return if the navigation will take place in a new window (if open() will
  /// be called)
  Status navigate(UriW uri, ICoreWebView2 *sender = nullptr,
                  std::function<void(ChWindow *newWindow)> open = nullptr,
                  bool &shown = *(bool *)nullptr);
};