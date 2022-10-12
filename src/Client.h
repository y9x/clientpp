#pragma once
#include "../utils/Uri.h"
#include "./AccountManager.h"
#include "./ClientFolder.h"
#include "./KrunkerWindow.h"
#include <discord_register.h>
#include <discord_rpc.h>

class Client {
private:
  AccountManager &accounts;
  ClientFolder &folder;
  KrunkerWindow game;
  KrunkerWindow social;
  KrunkerWindow editor;
  KrunkerWindow viewer;
  KrunkerWindow scripting;
  DiscordEventHandlers presence_events{};
  void rpc_loading();
  void install_runtimes();
  bool on_message(JSMessage msg, KrunkerWindow &window);
  void listen_navigation(KrunkerWindow &window);

public:
  Client(ClientFolder &folder, AccountManager &accounts);
  // Dispatch events to individual windows.
  void dispatch();
  /**
   * @param uri
   * @param sender Window to reference to determine if navigation should be
   * blocked.
   * @param open Called when the window is created/fetched. newWindow will be
   * a nullptr if the URL was opened via the shell
   * @return if the navigation will take place in a new window (if open() will
   * be called)
   */
  bool navigate(UriW uri, ICoreWebView2 *sender = nullptr,
                std::function<void(KrunkerWindow *newWindow)> open = nullptr);
};