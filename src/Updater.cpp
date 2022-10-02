#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./Updater.h"
#include "../utils/StringUtil.h"
#include "./Log.h"
#include <rapidjson/error/en.h>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <net.h>
#include <semver.hpp>

bool Updater::GetServing(UpdaterServing &serving)
{
	try
	{
		auto data = net::fetch_request(net::url(url));

		rapidjson::Document document;
		rapidjson::ParseResult ok = document.Parse(data.data(), data.size());

		if (!ok)
		{
			clog::error << "Error parsing serving: " << GetParseError_En(ok.Code()) << " (" << ok.Offset() << ")" << clog::endl;
			return false;
		}

		rapidjson::Document::Object client = document["client"].GetObj();

		serving.url = {
			client["url"].GetString(),
			client["url"].GetStringLength(),
		};

		serving.version = {
			client["semver"].GetString(),
			client["semver"].GetStringLength(),
		};

		return true;
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