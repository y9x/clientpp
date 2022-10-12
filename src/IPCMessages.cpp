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

void KrunkerWindow::handle_message(JSMessage msg) {
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

    call_create_webview(
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
    if (type == Type::Game && !seeking &&
        folder.config["game"]["seek"]["F4"].GetBool())
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

              mtx.lock();
              pending_navigations.push_back(ST::wstring(url));
              mtx.unlock();

              seeking = false;
            },
            JT::string(msg.args[0]));
      else {
        mtx.lock();
        pending_navigations.push_back(L"https://krunker.io/");
        mtx.unlock();
      }

    break;
  case IM::toggle_fullscreen:
    if (type != Type::Game)
      break;
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
      enter_fullscreen();
    else
      exit_fullscreen();
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

          mtx.lock();
          pending_messages.push_back(res);
          mtx.unlock();
        },
        msg);

    break;
  default:
    if (!on_unknown_message || !on_unknown_message(msg))
      clog::error << "Unknown message " << msg.dump() << clog::endl;

    break;
  }
}