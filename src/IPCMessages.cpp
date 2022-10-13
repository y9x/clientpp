#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "./KrunkerWindow.h"
#include "./LobbySeeker.h"
#include "./Log.h"
#include "./main.h"
#include "./resource.h"
#include <commdlg.h>
#include <rapidjson/prettywriter.h>
#include <shellapi.h>

/// @brief Full JSON presence support.
DiscordRichPresence loadPresenceData(const rapidjson::Value &value) {
  assert(value.IsObject());

  // This won't allocate any strings, instead it will use the pointers
  // referenced from the JSON data.
  DiscordRichPresence presence;
  memset(&presence, 0, sizeof(presence));

  presence.state =
      value.HasMember("state") ? value["state"].GetString() : nullptr;
  presence.details =
      value.HasMember("details") ? value["details"].GetString() : nullptr;
  presence.startTimestamp = value.HasMember("startTimestamp")
                                ? value["startTimestamp"].GetInt64()
                                : 0;
  presence.endTimestamp =
      value.HasMember("endTimestamp") ? value["endTimestamp"].GetInt64() : 0;
  presence.largeImageKey = value.HasMember("largeImageKey")
                               ? value["largeImageKey"].GetString()
                               : nullptr;
  presence.largeImageText = value.HasMember("largeImageText")
                                ? value["largeImageText"].GetString()
                                : nullptr;
  presence.smallImageKey = value.HasMember("smallImageKey")
                               ? value["smallImageKey"].GetString()
                               : nullptr;
  presence.smallImageText = value.HasMember("smallImageText")
                                ? value["smallImageText"].GetString()
                                : nullptr;
  presence.partyId =
      value.HasMember("partyId") ? value["partyId"].GetString() : nullptr;
  presence.partySize =
      value.HasMember("endTimestamp") ? value["endTimestamp"].GetInt64() : 0;
  presence.partyMax =
      value.HasMember("endTimestamp") ? value["endTimestamp"].GetInt64() : 0;
  presence.partyPrivacy =
      value.HasMember("endTimestamp") ? value["endTimestamp"].GetInt64() : 0;
  presence.matchSecret = value.HasMember("matchSecret")
                             ? value["matchSecret"].GetString()
                             : nullptr;
  presence.joinSecret =
      value.HasMember("joinSecret") ? value["joinSecret"].GetString() : nullptr;
  presence.spectateSecret = value.HasMember("spectateSecret")
                                ? value["spectateSecret"].GetString()
                                : nullptr;
  presence.instance =
      value.HasMember("instance") ? value["instance"].GetInt() : 0;

  return presence;
}

void ChScriptedWindow::handleMessage(JSMessage msg) {
  if (postedMessages.contains(msg.event)) {
    const auto &[then, catchError] = postedMessages[msg.event];

    if (msg.args.Size() == 2)
      catchError(msg.args[1]);
    else
      then(msg.args[0]);

    postedMessages.erase(msg.event);

    return;
  }

  switch (msg.event) {
  case IM::save_config: {
    folder.config.CopyFrom(msg.args[0], folder.config.GetAllocator());
    folder.save_config();
  } break;
  case IM::open_devtools:
    if (folder.config["client"]["devtools"].GetBool())
      webview->OpenDevToolsWindow();
    break;
  case IM::shell_open: {
    std::wstring open;

    if (msg.args[0] == "root")
      open = folder.directory;
    else if (msg.args[0] == "logs")
      open = folder.directory + folder.p_logs;
    else if (msg.args[0] == "scripts")
      open = folder.directory + folder.p_scripts;
    else if (msg.args[0] == "styles")
      open = folder.directory + folder.p_styles;
    else if (msg.args[0] == "swapper")
      open = folder.directory + folder.p_swapper;
    else if (msg.args[0] == "url")
      open = JT::wstring(msg.args[1]);

    ShellExecute(m_hWnd, L"open", open.c_str(), L"", L"", SW_SHOW);
  } break;
  case IM::pointer:
    last_pointer_poll = now();
    if (msg.args[0].GetBool() && !mouse_hooked)
      hook_mouse();
    else if (!msg.args[0].GetBool() && mouse_hooked)
      unhook_mouse();

    break;
  case IM::log: {
    std::string log = JT::string(msg.args[1]);

    switch (msg.args[0].GetInt()) {
    case LogType::info:
      clog::info << log << clog::endl;
      break;
    case LogType::warn:
      clog::warn << log << clog::endl;
      break;
    case LogType::error:
      clog::error << log << clog::endl;
      break;
    case LogType::debug:
      clog::debug << log << clog::endl;
      break;
    }
  } break;
  case IM::relaunch_webview: {
    wil::unique_cotaskmem_string uri;
    webview->get_Source(&uri);
    std::wstring uricopy(uri.get());

    control->Close();

    callCreateWebView(
        [this, uricopy]() { webview->Navigate(uricopy.c_str()); });
  } break;
  case IM::close_window:
    if (::IsWindow(m_hWnd))
      DestroyWindow();

    break;
  case IM::reload_window:
    webview->Stop();
    webview->Reload();

    break;
  case IM::seek_game:
    if (folder.config["game"]["seek"]["F4"].GetBool())
      if (folder.config["game"]["seek"]["custom_logic"].GetBool())
        new std::thread(
            [this](std::string sregion) {
              seeking = true;

              LobbySeeker seeker;

              for (size_t mi = 0; mi < LobbySeeker::modes.size(); mi++)
                if (LobbySeeker::modes[mi] ==
                    JT::string(folder.config["game"]["seek"]["mode"])) {
                  seeker.mode = mi;
                }

              for (size_t ri = 0; ri < LobbySeeker::regions.size(); ri++)
                if (LobbySeeker::regions[ri].first == sregion) {
                  seeker.region = ri;
                }

              seeker.customs =
                  folder.config["game"]["seek"]["customs"].GetBool();
              seeker.map = ST::lowercase(
                  JT::string(folder.config["game"]["seek"]["map"]));

              if (seeker.map.length())
                seeker.use_map = true;

              std::string url = seeker.seek();

              dispatchMtx.lock();
              pendingNavigations.push_back(ST::wstring(url));
              dispatchMtx.unlock();

              seeking = false;
            },
            JT::string(msg.args[0]));
      else {
        dispatchMtx.lock();
        pendingNavigations.push_back(L"https://krunker.io/");
        dispatchMtx.unlock();
      }

    break;
  case IM::toggle_fullscreen:
    folder.config["render"]["fullscreen"] =
        rapidjson::Value(!folder.config["render"]["fullscreen"].GetBool());
    folder.save_config();

    {
      JSMessage msg(IM::update_menu);

      msg.args.PushBack(rapidjson::Value(folder.config, msg.allocator),
                        msg.allocator);

      if (!sendMessage(msg))
        clog::error << "Unable to send " << msg.dump() << clog::endl;
    }
  case IM::fullscreen:
    if (folder.config["render"]["fullscreen"].GetBool())
      enterFullscreen();
    else
      exitFullscreen();
    break;
  case IM::update_meta:
    title = JT::wstring(folder.config["window"]["meta"]["title"]);
    SetIcon((HICON)LoadImage(
        NULL,
        folder
            .resolve_path(JT::wstring(folder.config["window"]["meta"]["icon"]))
            .c_str(),
        IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
    SetWindowText(title.c_str());

    break;
  case IM::revert_meta:
    title = og_title;
    SetIcon(getMainIcon());
    SetWindowText(title.c_str());

    break;
  case IM::reload_config:
    folder.load_config();

    break;
  case IM::browse_file:
    new std::thread(
        [this](JSMessage msg) {
          wchar_t filename[MAX_PATH];

          OPENFILENAME ofn;
          ZeroMemory(&filename, sizeof(filename));
          ZeroMemory(&ofn, sizeof(ofn));
          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = m_hWnd;
          std::wstring title = JT::wstring(msg.args[1]);
          std::wstring filters;

          for (rapidjson::Value::ValueIterator it = msg.args[2].Begin();
               it != msg.args[2].End(); ++it) {
            std::wstring wlabel = JT::wstring((*it)[0]);
            std::wstring wfilter = JT::wstring((*it)[1]);

            wlabel += L" (" + wfilter + L")";

            filters += wlabel;
            filters += L'\0';
            filters += wfilter;
            filters += L'\0';
          }

          // L"Icon Files\0*.ico\0Any File\0*.*\0"
          // filters is terminated by 2 null characters
          // each filter is terminated by 1 null character
          ofn.lpstrFilter = filters.c_str();
          ofn.lpstrFile = filename;
          ofn.nMaxFile = MAX_PATH;
          ofn.lpstrTitle = title.c_str();
          ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

          JSMessage res(msg.args[0].GetInt());

          if (GetOpenFileName(&ofn)) {
            std::wstring fn;
            fn.resize(MAX_PATH);
            fn = filename;

            // make relative
            std::string rel = ST::string(folder.relative_path(fn));
            res.args.PushBack(
                rapidjson::Value(rel.data(), rel.size(), res.allocator),
                res.allocator);
            res.args.PushBack(rapidjson::Value(false), res.allocator);
          } else
            res.args.PushBack(rapidjson::Value(true), res.allocator);

          dispatchMtx.lock();
          pendingMessages.push_back(res);
          dispatchMtx.unlock();
        },
        msg);

    break;
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

    if (!sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  case IM::account_remove: {

    accounts.data.erase(JT::string(msg.args[0]));
    accounts.save();

    JSMessage res(IM::account_regen);
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!sendMessage(res))
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
    if (!sendMessage(res))
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
      if (!sendMessage(res))
        clog::error << "Unable to send " << res.dump() << clog::endl;
    } else
      clog::error << "Failure encrypting password" << clog::endl;
  } break;
  case IM::account_list: {

    JSMessage res(msg.args[0].GetInt());
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  case IM::rpc_update:
    if (msg.args[0].IsNull()) {
      Discord_ClearPresence();
    } else {
      DiscordRichPresence data = loadPresenceData(msg.args[0]);
      Discord_UpdatePresence(&data);
    }

    break;
  default:

    clog::error << "Unknown message " << msg.dump() << clog::endl;

    break;
  }
}