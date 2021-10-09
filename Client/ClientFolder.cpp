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

ClientFolder::ClientFolder(std::wstring n) : name(n) {}

bool ClientFolder::create_directory(std::wstring directory) {
	bool result = CreateDirectory(directory.c_str(), NULL);

	if (result) {
		clog::info << "Created " << Convert::string(directory) << clog::endl;
		return true;
	}
	else if (GetLastError() == ERROR_ALREADY_EXISTS)return true;
	else {
		clog::error << "Unable to create " << Convert::string(directory) << clog::endl;
		return false;
	}
}

bool ClientFolder::create() {
	bool ret = true;
	
	directory = _wgetenv(L"USERPROFILE");
	directory += L"\\Documents\\" + name;

	if (create_directory(directory)) {
		if (write_resource(directory + p_guru, ICON_GURU)) clog::info << "Created " << Convert::string(directory + p_guru) << clog::endl;
		if (write_resource(directory + p_clientpp, ICON_CLIENTPP)) clog::info << "Created " << Convert::string(directory + p_clientpp) << clog::endl;
		
		for (std::wstring sdir : directories) {
			if (!create_directory(directory + sdir)) ret = false;
		}
		
		clog::logs = directory + p_logs;
	}
	else clog::error << "Unable to create root folder" << clog::endl, ret = false;

	std::string config_buffer;
	if (load_resource(JSON_CONFIG, config_buffer)) {
		default_config = JSON::parse(config_buffer);
		default_config["window"]["meta"]["icon"] = Convert::string(directory + p_clientpp);
	}

	return ret;
}

JSON empty_param;

JSON traverse_copy(JSON value, JSON match, bool& bad_key, JSON& obj_preset = empty_param) {
	if (value.is_object()) {
		JSON result = &obj_preset == &empty_param ? match : obj_preset;

		for (auto [skey, svalue] : value.items()) {
			if (match.contains(skey) && match[skey].type() == value[skey].type()) {
				result[skey] = traverse_copy(svalue, match[skey], bad_key);
			}
			else {
				bad_key = true;
				// clog::warn << skey << " from " << value << " does not match " << match << clog::endl;
			}
		}

		return result;
	}
	else {
		return value;
	}
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

	clog::debug << "Config loaded" << clog::endl;

	return true;
}

bool ClientFolder::save_config() {
	clog::debug << "Config saved" << clog::endl;
	return IOUtil::write_file(directory + p_config, config.dump(1, '\t'));
}