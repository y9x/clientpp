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

// document as "relative to config.json"
std::wstring ClientFolder::resolve_path(std::wstring file) {
	std::wstring joined = directory + L"\\" + file;

	// default_config["window"]["meta"]["icon"] = Convert::string(directory + p_krunker);

	FILE* f = _wfopen(joined.c_str(), L"r");
	if (f) {
		fclose(f);
		return joined;
	}

	return file;
}

std::wstring ClientFolder::relative_path(std::wstring path) {
	return Manipulate::replace_all(path, directory + L"\\", L"");
}

bool ClientFolder::create() {
	bool ret = true;
	
	directory = _wgetenv(L"USERPROFILE");
	directory += L"\\Documents\\" + name;

	if (create_directory(directory)) {
		if (write_resource(directory + p_chief, ICON_CHIEF)) clog::info << "Created " << Convert::string(directory + p_chief) << clog::endl;
		if (write_resource(directory + p_krunker, ICON_KRUNKER)) clog::info << "Created " << Convert::string(directory + p_krunker) << clog::endl;
		
		for (std::wstring sdir : directories) {
			if (!create_directory(directory + sdir)) ret = false;
		}
		
		clog::logs = directory + p_logs;
	}
	else clog::error << "Unable to create root folder" << clog::endl, ret = false;

	std::string config_buffer;
	if (load_resource(JSON_CONFIG, config_buffer)) {
		default_config = JSON::parse(config_buffer);
	}

	return ret;
}

JSON empty_param;

JSON traverse_copy(JSON value, JSON match, JSON& obj_preset = empty_param) {
	if (value.type() != match.type()) return match;

	if (value.is_object()) {
		JSON result = &obj_preset == &empty_param ? match : obj_preset;

		for (auto [skey, svalue] : value.items())
			if (match.contains(skey)) result[skey] = traverse_copy(svalue, match[skey]);
		
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

	bool used_default = false;

	try {
		new_config = JSON::parse(config_buffer);
	}
	catch (JSON::exception err) {
		used_default = true;
		new_config = default_config;
	}

	config = traverse_copy(new_config, default_config, default_config);

	clog::debug << "Config loaded" << clog::endl;
	
	save_config();

	return true;
}

bool ClientFolder::save_config() {
	if (IOUtil::write_file(directory + p_config, config.dump(1, '\t'))) {
		clog::debug << "Config saved" << clog::endl;
		return true;
	}
	else return false;
}