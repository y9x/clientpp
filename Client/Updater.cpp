#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.hpp>
#include <json.hpp>
#include "./Updater.h"
#include "./Log.h"

using JSON = nlohmann::json;

bool Updater::GetServing(UpdaterServing& serving) {
	httplib::Client cli(host);
	
	if (auto res = cli.Get(path.c_str()))
		try {
			JSON parsed = JSON::parse(res->body);
			JSON client = parsed["client"];
			
			serving.url = client["url"];
			serving.version = client["version"];

			return true;
		}
		catch (JSON::parse_error err) {
			clog::error << "Error parsing serving: " << err.what() << clog::endl;
		}
	else {
		httplib::Error error = res.error();
		clog::error << "Error checking updates: 0x" << std::hex << error << clog::endl;
	}

	return false;
}

bool Updater::UpdatesAvailable(UpdaterServing& serving) {
	if (!GetServing(serving)) return false;
	return version < serving.version;
}

Updater::Updater(long double d, std::string h, std::string p) : version(d), host(h), path(p) {}