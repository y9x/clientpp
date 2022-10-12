#define _CRT_SECURE_NO_WARNINGS
#include "./Client.h"
#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "./Log.h"
#include "./Site.h"
#include <sstream>

constexpr const char *discord_rpc = "899137303182716968";
constexpr const wchar_t *title = L"Chief Client";
constexpr const wchar_t *title_game = title;
constexpr const wchar_t *title_social = L"Social";
constexpr const wchar_t *title_editor = L"Editor";
constexpr const wchar_t *title_viewer = L"Viewer";
constexpr const wchar_t *title_scripting = L"Scripting";

bool Client::navigate(UriW uri, ICoreWebView2 *sender,
                      std::function<void(KrunkerWindow *newWindow)> open) {
  if (uri.host() == L"chief")
    return false;

  KrunkerWindow *send = nullptr;

  if (uri.host() == L"krunker.io") {
    if (uri.path() == krunker::game || uri.path().starts_with(krunker::games))
      send = &game;
    else if (uri.path() == krunker::social)
      send = &social;
    else if (uri.path() == krunker::editor)
      send = &editor;
    else if (uri.path() == krunker::viewer)
      send = &viewer;
    else if (uri.path() == krunker::scripting)
      send = &scripting;
  }

  if (!send) {
    ShellExecute(NULL, L"open", uri.toString().c_str(), L"", L"", SW_SHOW);
    if (open)
      open(nullptr);
    return true;
  } else if (!send->webview || send->webview != sender) {
    send->get([this, uri, send, open](bool newly_created) {
      if (newly_created)
        listen_navigation(*send);
      std::wstring uristr = uri.toString();
      send->webview->Navigate(uristr.c_str());
      send->BringWindowToTop();
      if (open)
        open(send);
    });
    return true;
  }

  return false;
}

void Client::listen_navigation(KrunkerWindow &window) {
  EventRegistrationToken token;

  window.webview->add_NewWindowRequested(
      Microsoft::WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NewWindowRequestedEventArgs *args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);

            ICoreWebView2 *newWindow = nullptr;

            wil::com_ptr<ICoreWebView2Deferral> defer;

            // args->AddRef();
            if (SUCCEEDED(args->GetDeferral(&defer)))
              if (!navigate(UriW(uri.get()), sender,
                            [defer, args](KrunkerWindow *newWindow) -> void {
                              if (newWindow)
                                args->put_NewWindow(newWindow->webview.get());
                              args->put_Handled(true);
                              defer->Complete();
                            }))
                defer->Complete();

            return S_OK;
          })
          .Get(),
      &token);

  window.webview->add_NavigationStarting(
      Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);
            if (navigate(UriW(uri.get()), sender))
              args->put_Cancel(true);

            return S_OK;
          })
          .Get(),
      &token);
}

void Client::rpc_loading() {
  DiscordRichPresence presence;
  memset(&presence, 0, sizeof(presence));

  presence.startTimestamp = time(0);
  presence.largeImageKey = "icon";

  presence.state = "Loading";

  Discord_UpdatePresence(&presence);
}

bool Client::on_message(JSMessage msg, KrunkerWindow &window) {
  switch (msg.event) {
  case IM::rpc_init:

    if (!folder.config["rpc"]["enabled"].GetBool())
      break;

    rpc_loading();

    break;
  case IM::rpc_clear:

    Discord_ClearPresence();

    break;
  case IM::rpc_update: {
    {
      if (!folder.config["rpc"]["enabled"].GetBool())
        break;

      DiscordRichPresence presence;
      memset(&presence, 0, sizeof(presence));

      int64_t start = msg.args[0].GetInt64();

      std::string user = JT::string(msg.args[1]);
      std::string map = JT::string(msg.args[2]);
      std::string mode = JT::string(msg.args[3]);

      presence.startTimestamp = start;
      presence.largeImageKey = "icon";

      presence.state = mode.c_str();
      presence.details = map.c_str();

      if (folder.config["rpc"]["name"].GetBool())
        presence.largeImageText = user.c_str();
      else
        presence.state = "In game";

      Discord_UpdatePresence(&presence);
    }
  } break;
  case IM::account_password: {
    JSMessage res(msg.args[0].GetInt());
    std::string dec;
    std::string account_name = JT::string(msg.args[1]);

    if (!accounts.data.contains(account_name)) {
      res.args.PushBack(rapidjson::Value(rapidjson::kNullType), res.allocator);
      res.args.PushBack(
          rapidjson::Value("Account doesn't exist", res.allocator),
          res.allocator);
    } else if (!accounts.decrypt(accounts.data[account_name].password, dec)) {
      res.args.PushBack(rapidjson::Value(rapidjson::kNullType), res.allocator);
      res.args.PushBack(rapidjson::Value("Unknown", res.allocator),
                        res.allocator);
    } else {
      res.args.PushBack(rapidjson::Value(dec.data(), dec.size(), res.allocator),
                        res.allocator);
    }

    if (!window.sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  case IM::account_remove: {

    accounts.data.erase(JT::string(msg.args[0]));
    accounts.save();

    JSMessage res(IM::account_regen);
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!window.sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  case IM::account_set: {

    std::string name = JT::string(msg.args[0]);
    Account &account = accounts.data[name];
    account.color = JT::string(msg.args[1]);
    account.order = msg.args[2].GetInt();
    accounts.save();

    JSMessage res(IM::account_regen);
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!window.sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  // and creation
  case IM::account_set_password: {

    std::string enc;
    if (accounts.encrypt(JT::string(msg.args[1]), enc)) {
      Account account;
      account.color = JT::string(msg.args[2]);
      account.order = msg.args[3].GetInt();
      account.username = JT::string(msg.args[0]);
      account.password = enc;
      accounts.data[account.username] = account;
      accounts.save();

      JSMessage res(IM::account_regen);
      res.args.PushBack(accounts.dump(res.allocator), res.allocator);
      if (!window.sendMessage(res))
        clog::error << "Unable to send " << res.dump() << clog::endl;
    } else
      clog::error << "Failure encrypting password" << clog::endl;
  } break;
  case IM::account_list: {

    JSMessage res(msg.args[0].GetInt());
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!window.sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;

  default:
    return false;
  }

  return true;
}

Client::Client(ClientFolder &_folder, AccountManager &_accounts)
    : accounts(accounts), folder(_folder),
      game(
          folder, KrunkerWindow::Type::Game, {0.8, 0.8}, title_game,
          [this]() { listen_navigation(game); },
          [this](JSMessage msg) -> bool { return on_message(msg, game); },
          []() { PostQuitMessage(EXIT_SUCCESS); }),
      social(
          folder, KrunkerWindow::Type::Social, {0.7, 0.7}, title_social,
          [this]() { listen_navigation(social); },
          [this](JSMessage msg) -> bool { return on_message(msg, social); }),
      editor(
          folder, KrunkerWindow::Type::Editor, {0.7, 0.7}, title_editor,
          [this]() { listen_navigation(editor); },
          [this](JSMessage msg) -> bool { return on_message(msg, editor); }),
      viewer(
          folder, KrunkerWindow::Type::Viewer, {0.4, 0.6}, title_viewer,
          [this]() { listen_navigation(viewer); },
          [this](JSMessage msg) -> bool { return on_message(msg, viewer); }),
      scripting(
          folder, KrunkerWindow::Type::Scripting, {0.6, 0.6}, title_scripting,
          [this]() { listen_navigation(scripting); },
          [this](JSMessage msg) -> bool {
            return on_message(msg, scripting);
          }) {
  memset(&presence_events, 0, sizeof(presence_events));
  Discord_Initialize(discord_rpc, &presence_events, 1, NULL);

  if (folder.config["rpc"]["enabled"].GetBool())
    rpc_loading();

  HRESULT coinit = CoInitialize(NULL);
  if (!SUCCEEDED(coinit))
    MessageBox(NULL,
               (L"COM could not be initialized. CoInitialize returned " +
                ST::wstring(std::to_string(coinit)))
                   .c_str(),
               title, MB_OK);

  clog::info << "Main initialized" << clog::endl;

  game.can_fullscreen = true;
}

void Client::dispatch() {
  game.dispatch();
  social.dispatch();
  editor.dispatch();
  viewer.dispatch();
  scripting.dispatch();
}