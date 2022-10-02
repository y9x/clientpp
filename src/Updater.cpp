#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <net.h>
#include <json.hpp>
#include <semver.hpp>
#include "../utils/StringUtil.h"
#include "./Updater.h"
#include "./Log.h"

using JSON = nlohmann::json;

bool Updater::GetServing(UpdaterServing &serving)
{
	try
	{
		auto data = net::fetch_request(net::url(url));

		JSON parsed = JSON::parse(std::string(data.begin(), data.end()));
		JSON client = parsed["client"];

		serving.url = client["url"];
		serving.version = client["semver"];

		return true;
	}
	catch (JSON::parse_error err)
	{
		clog::error << "Error parsing serving: " << err.what() << clog::endl;
		return false;
	}
	catch (net::error &err)
	{
		clog::error << "Error checking updates: " << err.what() << clog::endl;
		return false;
	}
}

bool Updater::UpdatesAvailable(UpdaterServing &serving)
{
	if (!GetServing(serving))
		return false;
	return semver::version(version) < semver::version(serving.version);
}

Updater::Updater(std::string v, std::wstring u) : version(v), url(u) {}