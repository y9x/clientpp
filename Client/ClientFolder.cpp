#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "./ClientFolder.h"
#include "../Utils/StringUtil.h"
#include "../Utils/IOUtil.h"
#include "./Log.h"
#include "./LoadRes.h"
#include "resource.h"

using namespace StringUtil;

// true if the result of CreateDirectory is nonzero or if GetLastError equals ERROR_ALREADY_EXISTS, otherwise false
bool OVR(int result) {
	if (result != 0)return true;
	else if (GetLastError() == ERROR_ALREADY_EXISTS)return true;
	else return false;
}

JSON traverse_copy(JSON value, JSON match, bool& bad_key, JSON result = JSON::object()) {
	if (value.is_object()) {
		for (auto [skey, svalue] : value.items()) {
			if (match.contains(skey)) {
				result[skey] = traverse_copy(svalue, match[skey], bad_key);
			}
			else {
				bad_key = true;
				LOG_WARN(skey << " from " << value << " does not match " << match);
			}
		}
	}
	else {
		result = value;
	}

	return result;
}

bool write_resource(std::wstring path, int resource) {
	HRSRC src = FindResource(NULL, MAKEINTRESOURCE(resource), RT_RCDATA);
	bool ret = false;

	if (src != NULL) {
		HGLOBAL header = LoadResource(NULL, src);
		if (header != NULL) {
			void* data = (char*)LockResource(header);

			if (data != NULL) {
				HANDLE file = CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

				if (file != INVALID_HANDLE_VALUE) {
					DWORD size = SizeofResource(0, src);
					DWORD bytes;
					
					WriteFile(file, data, size, &bytes, nullptr);
					CloseHandle(file);

					ret = true;
				}
			}
			UnlockResource(header);
		}

		FreeResource(header);
	}

	return ret;
}

ClientFolder::ClientFolder(std::wstring n) : name(n) {
	directory = _wgetenv(L"USERPROFILE");
	directory += L"\\Documents\\" + name;

	if (OVR(CreateDirectory(directory.c_str(), NULL))) {
		LOG_INFO("Created " << Convert::string(directory));

		if (write_resource(directory + p_guru, ICON_GURU)) LOG_INFO("Created " << Convert::string(directory + p_guru));
		if (write_resource(directory + p_clientpp, ICON_CLIENTPP)) LOG_INFO("Created " << Convert::string(directory + p_clientpp));

		for (std::wstring sdir : directories) {
			if (OVR(CreateDirectory((directory + sdir).c_str(), NULL))) LOG_INFO("Created " << Convert::string(directory + sdir));
			else error_creating = true;
		}
		
		fc.path = directory + p_log;
	}
	else LOG_ERROR("Creation"), error_creating = true;

	std::string config_buffer;
	if (load_resource(JSON_CONFIG, config_buffer)) {
		default_config = JSON::parse(config_buffer);
		default_config["window"]["meta"]["icon"] = Convert::string(directory + p_clientpp);
	}

	if (error_creating)LOG_ERROR("Had an error creating directories");
	else load_config();
}


bool ClientFolder::load_config() {
	std::string config_buffer;

	IOUtil::read_file(directory + p_config, config_buffer);

	JSON new_config = JSON::object();
	bool save = false;

	try {
		new_config = JSON::parse(config_buffer);
	}
	catch (JSON::exception err) {
		new_config = default_config;
		save = true;
	}

	config = traverse_copy(new_config, default_config, save, default_config);

	if (save) save_config();

	return true;
}

bool ClientFolder::save_config() {
	LOG_INFO("Wrote config");
	return IOUtil::write_file(directory + p_config, config.dump(1, '\t'));
}