#include "./Updater.h"
#include "../utils/StringUtil.h"
#include "./Log.h"
#include "./fetch.h"
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <semver.hpp>

bool getServing(const std::string &url, UpdaterServing &serving) {
  try {
    auto data = fetchGet(url);

    rapidjson::Document document;
    rapidjson::ParseResult ok = document.Parse(data.data(), data.size());

    if (!ok) {
      clog::error << "Error parsing serving: " << GetParseError_En(ok.Code())
                  << " (" << ok.Offset() << ")" << clog::endl;
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
  } catch (const std::runtime_error &e) {
    clog::error << "Failure checking updates. " << e.what() << clog::endl;
    return false;
  }
}

bool updatesAvailable(const std::string &version, const std::string &url,
                      UpdaterServing &serving) {
  if (!getServing(url, serving))
    return false;
  return semver::version(version) < semver::version(serving.version);
}
