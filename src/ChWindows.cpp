#include "./KrunkerWindow.h"

constexpr const char *discordRPC = "899137303182716968";
constexpr const wchar_t *title = L"Chief Client";
constexpr const wchar_t *titleGame = title;
constexpr const wchar_t *titleSocial = L"Social";
constexpr const wchar_t *titleEditor = L"Editor";
constexpr const wchar_t *titleViewer = L"Viewer";
constexpr const wchar_t *titleScripting = L"Scripting";

ChWindow *ChWindows::getWindow(krunker::type type) {
  switch (type) {
  case krunker::Game:
    return &game;
  case krunker::Social:
    return &social;
  case krunker::Editor:
    return &editor;
  case krunker::Viewer:
    return &viewer;
  case krunker::type::Scripting:
    return &scripting;
  default:
    return nullptr;
  }
}

ChWindows::ChWindows(ClientFolder &folder, AccountManager &accounts)
    : game(folder, accounts, *this, {0.8, 0.8}, titleGame),
      social(folder, accounts, *this, {0.8, 0.8}, titleSocial),
      editor(folder, *this, {0.8, 0.8}, titleEditor),
      viewer(folder, *this, {0.8, 0.8}, titleViewer),
      scripting(folder, *this, {0.8, 0.8}, titleScripting) {}

Status ChWindows::navigate(UriW uri, ICoreWebView2 *sender,
                           std::function<void(ChWindow *newWindow)> open,
                           bool &shown) {
  if (uri.host() == L"chief.krunker.io") {
    if (&shown)
      shown = false;

    return Status::Ok;
  }

  ChWindow *window = getWindow(krunker::identifyType(uri));

  if (!window) {
    ShellExecute(NULL, L"open", uri.toString().c_str(), L"", L"", SW_SHOW);
    if (open)
      open(nullptr);
    return Status::Ok;
  }

  return window->show(
      uri, sender,
      [window, open]() -> void {
        if (open)
          open(window);
      },
      shown);
}

void ChWindows::dispatch() {
  game.dispatch();
  social.dispatch();
  editor.dispatch();
  viewer.dispatch();
  scripting.dispatch();
}
