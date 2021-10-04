#include <Windows.h>
#include "./LoadRes.h"

bool load_resource(int resource, std::string& string) {
	bool ret = false;
	HRSRC src = FindResource(NULL, MAKEINTRESOURCE(resource), RT_RCDATA);

	if (src != NULL) {
		HGLOBAL header = LoadResource(NULL, src);
		if (header != NULL) {
			char* data = (char*)LockResource(header);

			if (data != NULL) {
				string = data;
				string.resize(SizeofResource(0, src));
				ret = true;
			}

			UnlockResource(header);
		}

		FreeResource(header);
	}

	return ret;
}