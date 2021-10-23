#pragma once
#include <json.hpp>

nlohmann::json TraverseCopy(nlohmann::json value, nlohmann::json match, nlohmann::json* obj_preset = nullptr);