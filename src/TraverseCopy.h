#pragma once
#include <nlohmann/json.hpp>

nlohmann::json TraverseCopy(nlohmann::json value, nlohmann::json match,
                            bool allow_new_props = false,
                            bool *changed = nullptr);