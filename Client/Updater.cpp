#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./httplib.h"
#include "./Updater.h"
#include "./Log.h"

JSON Updater::GetServing() {
	httplib::Client cli(host);
	auto res = cli.Get(path.c_str());
	
	try {
		return JSON::parse(res->body);
	}
	catch (JSON::parse_error err) {
		clog::error << "Error parsing serving: " << err.what() << clog::endl;
		return JSON::object();
	}
}

bool Updater::UpdatesAvailable(std::string& url) {
	JSON serve = GetServing();
	url = serve["client"]["url"].get<std::string>();
	return version < serve["client"]["version"].get<double>();
}

Updater::Updater(long double d, std::string h, std::string p) : version(d), host(h), path(p) {}