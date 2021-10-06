#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./httplib.h"
#include "./Updater.h"

JSON Updater::GetServing() {
	httplib::Client cli(host);
	auto res = cli.Get(path.c_str());
	return JSON::parse(res->body);
}

bool Updater::UpdatesAvailable(std::string& url) {
	JSON serve = GetServing();

	url = serve["client"]["url"].get<std::string>();

	return version < serve["client"]["version"].get<double>();
}

Updater::Updater(long double d, std::string h, std::string p) : version(d), host(h), path(p) {}