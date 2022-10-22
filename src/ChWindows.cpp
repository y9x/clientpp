#include "./ChGameWindow.h"
#include "./ChScriptedWindow.h"
#include "./ChWindow.h"

constexpr const wchar_t *title = L"Chief Client";
constexpr const wchar_t *titleGame = title;
constexpr const wchar_t *titleSocial = L"Social";
constexpr const wchar_t *titleEditor = L"Editor";
constexpr const wchar_t *titleViewer = L"Viewer";
constexpr const wchar_t *titleScripting = L"Scripting";

ChWindow *ChWindows::getWindow(krunker::type type) {
  switch (type) {
  case krunker::Game:
    return game;
  case krunker::Social:
    return social;
  case krunker::Editor:
    return editor;
  case krunker::Viewer:
    return viewer;
  case krunker::type::Scripting:
    return scripting;
  default:
    return nullptr;
  }
}

ChWindows::ChWindows(ClientFolder &folder, AccountManager &accounts)
    : game(new ChGameWindow(folder, accounts, *this, {0.8, 0.8}, titleGame)),
      social(new ChScriptedWindow(folder, *this, {0.8, 0.8}, titleSocial)),
      editor(new ChScriptedWindow(folder, *this, {0.8, 0.8}, titleEditor)),
      viewer(new ChScriptedWindow(folder, *this, {0.8, 0.8}, titleViewer)),
      scripting(
          new ChScriptedWindow(folder, *this, {0.8, 0.8}, titleScripting)) {}

ChWindows::~ChWindows() {
  delete game;
  delete social;
  delete editor;
  delete viewer;
  delete scripting;
}

void ChWindows::dispatch() {
  game->dispatch();
  social->dispatch();
  editor->dispatch();
  viewer->dispatch();
  scripting->dispatch();
}

ChWindow::Status
ChWindows::navigate(UriW uri, ICoreWebView2 *sender,
                    std::function<void(ChWindow *newWindow)> open,
                    bool *shown) {
  if (uri.host() == L"chief.krunker.io") {
    if (shown)
      *shown = false;

    return ChWindow::Status::Ok;
  }

  ChWindow *window = getWindow(krunker::identifyType(uri));

  if (!window) {
    ShellExecute(NULL, L"open", uri.toString().c_str(), L"", L"", SW_SHOW);
    if (open)
      open(nullptr);
    return ChWindow::Status::Ok;
  }

  return window->show(
      uri, sender,
      [window, open]() -> void {
        if (open)
          open(window);
      },
      shown);
}

bool ChWindows::shouldQuit() {
  return !game->open && !social->open && !editor->open && !viewer->open &&
         !scripting->open;
}
