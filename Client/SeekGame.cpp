/*
// temp
#include <httplib.hpp>
#include "./KrunkerWindow.h"
#include "./Log.h"

using JSON = nlohmann::json;

httplib::Client api("https://matchmaker.krunker.io");

std::vector<std::string> modes = {"Free for All","Team Deathmatch","Hardpoint","Capture the Flag","Parkour","Hide & Seek","Infected","Race","Last Man Standing","Simon Says","Gun Game","Prop Hunt","Boss Hunt","unused","unused","Stalker","King of the Hill","One in the Chamber","Trade","Kill Confirmed","Defuse","Sharp Shooter","Traitor","Raid","Blitz","Domination","Squad Deathmatch","Kranked FFA"};

bool KrunkerWindow::seek_game() {
	std::string seek_mode = folder->config["game"]["seek"]["mode"];
	
	if (seek_mode != "any") {
		if(auto res = api.Get("/game-list?hostname=krunker.io"))try {
			JSON data = JSON::parse(res->body);

			for (JSON game : data["games"]) {
				JSON meta = game[4];
				int imode = meta["g"];
				
				if (modes.size() > imode) {
					std::string mode = modes[imode];

					if (mode == seek_mode) {

						break;
					}
				}
			}
		}
		catch (JSON::parse_error err) {
			clog::error << "Unable to parse game list: " << err.what() << clog::endl;
		}
		catch (JSON::type_error err) {
			clog::error << "Unable to process game list: " << err.what() << clog::endl;
		}
	}
	return SUCCEEDED(webview->Navigate((L"https://krunker.io" + pathname).c_str()));
}
*/

#include "./KrunkerWindow.h"

bool KrunkerWindow::seek_game() {
	return SUCCEEDED(webview->Navigate((L"https://krunker.io" + pathname).c_str()));
}