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
#include <discord_register.h>
#include <discord_rpc.h>
#include <functional>
#include <map>
#include <mutex>
#include <rapidjson/fwd.h>
#include <regex>
#include <string>
#include <wil/com.h>
#include <wrl.h>

long long now();

class ChWindows;

class ChWindow : public CWindowImpl<ChWindow> {
public:
  static HRESULT getLastHError();

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

private:
  std::wstring cmdLine();
  LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);

protected:
  static HRESULT lastHError;
  static void setLastHError(HRESULT result);

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

  std::map<short, std::pair<std::function<void(const rapidjson::Value &)>,
                            std::function<void(const rapidjson::Value &)>>>
      postedMessages;

  // Fullscreen:
  RECT saved_size;
  DWORD saved_style = 0;
  DWORD saved_ex_style = 0;
  bool fullscreen = false;
  bool enterFullscreen();
  bool exitFullscreen();
  std::vector<JSMessage> pendingMessages;

  std::wstring genericJS;

  // Messaging:

  /// @brief Used by runtimeData
  rapidjson::Value getUserScripts(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);

  // Window:

  /// @brief Send a resource defined in Resource.rc in response to a
  /// WebResourceRequestedEvent
  /// @param args WebResourceRequestedEvent args
  /// @param resource Resource ID
  /// @param mime Mime type
  /// @return If the operation was successful
  bool sendResource(ICoreWebView2WebResourceRequestedEventArgs *args,
                    int resource, std::wstring mime);
  virtual Status create(std::function<void()> callback = nullptr) override;

protected:
  /// @brief Handle a message sent from the frontend
  /// @param msg
  virtual void handleMessage(JSMessage msg);

  virtual void registerEvents() override;

  /// @brief Ran as soon as the DOM is available and the domain is krunker.io
  virtual void injectJS();

  /// @brief Produce runtime data
  /// format.
  /// @return UserScripts, UserStyles, and config in JSON format
  std::string runtimeData();

  /// @brief Used by runtimeData
  virtual rapidjson::Value getUserStyles(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);

  /// @brief Asynchronously sends a message and expects a result.
  /// @param msg
  /// @param then
  /// @param catchError
  /// @return
  bool postMessage(const JSMessage &msg,
                   std::function<void(const rapidjson::Value &)> then,
                   std::function<void(const rapidjson::Value &)> catchError);
  /// @brief Asynchronously sends a message.
  /// @param message
  /// @return
  bool sendMessage(const JSMessage &message);

public:
  ChScriptedWindow(ClientFolder &folder, ChWindows &windows, Vector2 scale,
                   std::wstring title);
  /// @brief Execute any pending operations from the main thread
  virtual void dispatch() override;
};

class ChGameWindow : public ChScriptedWindow {
private:
  // Window-Frontend messaging:

  AccountManager &accounts;

  // Mouse hooking:

  static LRESULT CALLBACK mouseMessage(int code, WPARAM wParam, LPARAM lParam);
  LRESULT on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  void hookMouse();
  void unhookMouse();
  RAWINPUTDEVICE rawInput;
  HHOOK mouseHook = 0;
  bool mouseHooked = false;
  std::time_t last_pointer_poll;

  long long mouse_hz = 244;
  long long mouse_interval = 1000 / mouse_hz;
  long long then = now();
  Vector2 movebuffer;

  std::string gameCSS1;
  std::string gameCSS2;
  std::wstring gameJS;

  bool seeking = false;
  void seekGame();

protected:
  // game CSS
  virtual rapidjson::Value getUserStyles(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator)
      override;

  // account messages
  virtual void handleMessage(JSMessage msg) override;

  // game.js
  virtual void injectJS() override;

  // mouse hooks
  virtual void registerEvents() override;

  BEGIN_MSG_MAP(ChGameWindow)
  CHAIN_MSG_MAP(ChWindow)
  MESSAGE_HANDLER(WM_INPUT, on_input)
  END_MSG_MAP()
public:
  // init resources
  ChGameWindow(ClientFolder &folder, AccountManager &accounts,
               ChWindows &windows, Vector2 scale, std::wstring title);
  // release hook
  ~ChGameWindow();
  // pointer poll
  virtual void dispatch() override;
};

class ChWindows {
private:
  ChGameWindow game;
  ChScriptedWindow social;
  ChScriptedWindow editor;
  ChScriptedWindow viewer;
  ChScriptedWindow scripting;

public:
  /// @brief Execute any pending operations from the main thread
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
  ChWindow::Status
  navigate(UriW uri, ICoreWebView2 *sender = nullptr,
           std::function<void(ChWindow *newWindow)> open = nullptr,
           bool &shown = *(bool *)nullptr);
};