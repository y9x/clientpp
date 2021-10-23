#include "./TraverseCopy.h"
#include "Log.h"

using JSON = nlohmann::json;
JSON TraverseCopy(JSON value, JSON match, JSON* obj_preset, bool allow_new_props, bool* changed) {
	if (value.type() != match.type()) {
		if (changed) clog::info << "mismatch" << clog::endl, *changed = true;
		return match;
	}

	if (value.is_object()) {
		JSON result = match;

		if (obj_preset) result = *obj_preset;

		for (auto [skey, svalue] : value.items()) {
			// clog::info << skey << " : " << svalue << clog::endl;
			if (match.contains(skey)) result[skey] = TraverseCopy(svalue, match[skey], nullptr, allow_new_props, changed);
			else if (allow_new_props) result[skey] = value[skey];
			else if (changed) *changed = true;
		}

		return result;
	}
	else {
		return value;
	}
}