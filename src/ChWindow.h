#pragma once
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
#include <memory>
#include <mutex>
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
  // todo: never overwrite ogTitle & title
  std::wstring title;
  std::wstring ogTitle;
  std::mutex dispatchMtx;
  virtual void registerEvents();
  Status callCreateWebView(std::function<void()> callback = nullptr);
  wil::com_ptr<ICoreWebView2Controller> control;
  wil::com_ptr<ICoreWebView2> webview;
  wil::com_ptr<ICoreWebView2Environment> env;
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
  bool open = false;
  virtual void dispatch();
  Status show(UriW uri, ICoreWebView2 *sender = nullptr,
              std::function<void()> open = nullptr, bool *shown = nullptr);
  ChWindow(ClientFolder &folder, ChWindows &windows, Vector2 scale,
           std::wstring title);
  ~ChWindow();
};

class ChWindows {
private:
  ChWindow *game;
  ChWindow *social;
  ChWindow *editor;
  ChWindow *viewer;
  ChWindow *scripting;

public:
  /// @brief Execute any pending operations from the main thread
  void dispatch();
  /// @brief Should be called after handling messages. Checks if all the windows
  /// are closed.
  bool shouldQuit();
  ChWindows(ClientFolder &folder, AccountManager &accounts);
  ~ChWindows();
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
           bool *shown = nullptr);
};