#include "./TraverseCopy.h"
#include "./Log.h"

using JSON = nlohmann::json;

JSON TraverseCopy(JSON value, JSON match, bool allow_new_props, bool* changed) {
	if (value.type() != match.type()) {
		if (changed) *changed = true;
		return match;
	}

	if (value.is_object()) {
		JSON result = match;

		for (auto [skey, svalue] : value.items()) {
			if (match.contains(skey)) result[skey] = TraverseCopy(svalue, match[skey], allow_new_props, changed);
			else if (allow_new_props && !result.contains(skey)) {
				result[skey] = svalue;
				// if (changed) *changed = true;
			}
			else if (changed) *changed = true;
		}

		return result;
	}
	else {
		return value;
	}
}