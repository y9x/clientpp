#define _CRT_SECURE_NO_WARNINGS
#include "./AccountManager.h"
#include "../Utils/IOUtil.h"
#include "../Utils/Base64.h"
#include <dpapi.h>

using JSON = nlohmann::json;

std::vector<std::string> AccountManager::list() {
	std::vector<std::string> list;
	for (auto [key, value] : data.items()) list.push_back(key);
	return list;
}

bool AccountManager::encrypt(std::string input, std::string& output) {
	DATA_BLOB in;
	DATA_BLOB out;

	in.pbData = (BYTE*)input.data();
	in.cbData = input.size();

	if (CryptProtectData(&in, NULL, NULL, NULL, NULL, 0, &out)) {
		std::string encrypted;
		encrypted.resize(out.cbData);
		memcpy(encrypted.data(), out.pbData, out.cbData);
		
		output = Base64::Encode(encrypted);

		return true;
	}

	return false;
}

bool AccountManager::decrypt(std::string input, std::string& output) {
	std::string encrypted = Base64::Decode(input);

	DATA_BLOB in;
	DATA_BLOB out;

	in.pbData = (BYTE*)encrypted.data();
	in.cbData = encrypted.size();

	if (CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out)) {
		output.resize(out.cbData); 
		memcpy(output.data(), out.pbData, out.cbData);

		return true;
	}

	return false;
}

bool AccountManager::save() {
	return IOUtil::write_file(folder->directory + path, data.dump());
}

bool AccountManager::load() {
	std::string read;
	if (IOUtil::read_file(folder->directory + path, read))try {
		data = JSON::parse(read);
	}
	catch (JSON::parse_error err) {}

	if (!data.is_object()) data = nlohmann::json::object();

	return true;
}

bool AccountManager::set(std::string name, std::string password) {
	std::string enc;
	if (!encrypt(password, enc)) return false;
	data[name] = enc;
	return save();
}

bool AccountManager::remove(std::string name) {
	data.erase(name);
	return save();
}

bool AccountManager::get(std::string name, std::string& password) {
	if (!data[name].is_string()) return false;
	std::string enc = data[name];
	return decrypt(enc, password);
}

AccountManager::AccountManager(ClientFolder& f) : folder(&f) {}
