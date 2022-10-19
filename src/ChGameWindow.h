#pragma once
#include "./ChScriptedWindow.h"

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

  std::mutex seeking;
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
