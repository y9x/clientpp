#include "./KrunkerWindow.h"
#include "../Utils/StringUtil.h"
#include "./Log.h"
#include "./resource.h"
#include "./LobbySeeker.h"
#include <shellapi.h>
#include <commdlg.h>

using JSON = nlohmann::json;
using namespace StringUtil;

void KrunkerWindow::handle_message(JSMessage msg) {
	switch (msg.event) {
	case IM::save_config:
		folder->config = msg.args[0];
		folder->save_config();

		break;
	case IM::open_devtools:
		if (folder->config["client"]["devtools"]) webview->OpenDevToolsWindow();
		break;
	case IM::shell_open: {
		std::wstring open;

		if (msg.args[0] == "root")open = folder->directory;
		else if (msg.args[0] == "logs") open = folder->directory + folder->p_logs;
		else if (msg.args[0] == "scripts") open = folder->directory + folder->p_scripts;
		else if (msg.args[0] == "styles") open = folder->directory + folder->p_styles;
		else if (msg.args[0] == "swapper") open = folder->directory + folder->p_swapper;
		else if (msg.args[0] == "url") open = Convert::wstring(msg.args[1].get<std::string>());

		ShellExecute(m_hWnd, L"open", open.c_str(), L"", L"", SW_SHOW);
	} break;
	case IM::pointer:
		last_pointer_poll = now();
		if (msg.args[0] && !mouse_hooked) hook_mouse();
		else if (!msg.args[0] && mouse_hooked) unhook_mouse();

		break;
	case IM::log: {
		std::string log = msg.args[1];

		switch (msg.args[0].get<int>()) {
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
	case IM::relaunch_webview:
		control->Close();
		call_create_webview();

		break;
	case IM::close_window:
		if (::IsWindow(m_hWnd)) DestroyWindow();

		break;
	case IM::reload_window:
		webview->Stop();
		webview->Reload();

		break;
	case IM::seek_game:
		if (type == Type::Game && !seeking && folder->config["game"]["seek"]["F4"]) new std::thread([this](std::string sregion) {
			seeking = true;

			LobbySeeker seeker;

			for (size_t mi = 0; mi < LobbySeeker::modes.size(); mi++) if (LobbySeeker::modes[mi] == folder->config["game"]["seek"]["mode"]) {
				seeker.mode = mi;
			}
			
			for (size_t ri = 0; ri < LobbySeeker::regions.size(); ri++) if (LobbySeeker::regions[ri].first == sregion) {
				seeker.region = ri;
			}
			
			seeker.customs = folder->config["game"]["seek"]["customs"];
			seeker.map = Manipulate::lowercase(folder->config["game"]["seek"]["map"]);

			if (seeker.map.length()) seeker.use_map = true;
			
			std::string url = seeker.seek();

			mtx.lock();
			pending_navigations.push_back(Convert::wstring(url));
			mtx.unlock();

			seeking = false;
		}, msg.args[0]);

		break;
	case IM::toggle_fullscreen:
		if (type != Type::Game) break;
		folder->config["render"]["fullscreen"] = !folder->config["render"]["fullscreen"];
		folder->save_config();

		{
			JSMessage msg(IM::update_menu);
			msg.args.push_back(folder->config);
			if (!msg.send(webview))clog::error << "Unable to send " << msg.dump() << clog::endl;
		}
	case IM::fullscreen:
		if (folder->config["render"]["fullscreen"]) enter_fullscreen();
		else exit_fullscreen();
		break;
	case IM::update_meta:
		title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());
		SetIcon((HICON)LoadImage(get_hinstance(), folder->resolve_path(Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>())).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		SetWindowText(title.c_str());

		break;
	case IM::revert_meta:
		title = og_title;
		SetIcon(LoadIcon(get_hinstance(), MAKEINTRESOURCE(MAINICON)));
		SetWindowText(title.c_str());

		break;
	case IM::reload_config:
		folder->load_config();

		break;
	case IM::browse_file:
		new std::thread([this](JSMessage msg) {
			wchar_t filename[MAX_PATH];

			OPENFILENAME ofn;
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = m_hWnd;
			std::wstring title = Convert::wstring(msg.args[1]);
			std::wstring filters;
			for (JSON value : msg.args[2]) {
				std::string label = value[0];
				std::string filter = value[1];

				label += " (" + filter + ")";

				filters += Convert::wstring(label);
				filters += L'\0';
				filters += Convert::wstring(filter);
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

			JSMessage res(msg.args[0].get<int>());

			if (GetOpenFileName(&ofn)) {
				std::wstring fn;
				fn.resize(MAX_PATH);
				fn = filename;

				// make relative
				res.args[0] = Convert::string(folder->relative_path(fn));
				res.args[1] = false;
			}
			else res.args[1] = true;

			mtx.lock();
			pending_messages.push_back(res);
			mtx.unlock();
		}, msg);

		break;
	default:
		if (!on_unknown_message || !on_unknown_message(msg)) clog::error << "Unknown message " << msg.dump() << clog::endl;

		break;
	}
}