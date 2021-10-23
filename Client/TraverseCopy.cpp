#include "./TraverseCopy.h"

using JSON = nlohmann::json;

JSON TraverseCopy(JSON value, JSON match, JSON* obj_preset) {
	if (value.type() != match.type()) return match;

	if (value.is_object()) {
		JSON result = match;

		if (obj_preset) result = *obj_preset;

		for (auto [skey, svalue] : value.items())
			if (match.contains(skey)) result[skey] = TraverseCopy(svalue, match[skey]);

		return result;
	}
	else {
		return value;
	}
}