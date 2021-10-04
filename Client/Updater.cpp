#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./httplib.h"
#include "./Updater.h"

Updater::Updater(long double d) : version(d) {}

JSON Updater::GetServing() {
	httplib::Client cli("https://y9x.github.io");
	auto res = cli.Get("/userscripts/serve.json");
	return JSON::parse(res->body);
}

bool Updater::UpdatesAvailable(std::string& url) {
	JSON serve = GetServing();

	url = serve["client"]["url"].get<std::string>();

	return version < serve["client"]["version"].get<double>();
}