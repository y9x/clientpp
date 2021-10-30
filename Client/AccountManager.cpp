#define _CRT_SECURE_NO_WARNINGS
#include "./AccountManager.h"
#include "../Utils/IOUtil.h"
#include "../Utils/Base64.h"
#include <dpapi.h>

using JSON = nlohmann::json;

JSON Account::dump() {
	JSON output = JSON::object();
	output["order"] = order;
	output["color"] = color;
	output["password"] = password;
	return output;
}

Account::Account(JSON data) {
	order = data["order"];
	color = data["color"];
	password = data["password"];
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

JSON AccountManager::dump() {
	JSON output = JSON::object();
	
	for (auto [name, acc] : data) data[name] = acc.dump();

	return output;
}

bool AccountManager::save() {
	return IOUtil::write_file(folder->directory + path, dump());
}

bool AccountManager::load() {
	std::string read;
	if (IOUtil::read_file(folder->directory + path, read))try {
		JSON parsed = JSON::parse(read);
		for (auto [name, data] : parsed.items()) data[name] = data;
	}
	catch (JSON::parse_error err) {}

	return true;
}

bool AccountManager::set(std::string name, std::string password) {
	std::string enc;
	if (!encrypt(password, enc)) return false;
	data[name].password = enc;
	return save();
}

bool AccountManager::remove(std::string name) {
	
	return save();
}

bool AccountManager::get(std::string name, std::string& password) {
	if (!data.contains(name)) return false;
	return decrypt(data[name].password, password);
}

AccountManager::AccountManager(ClientFolder& f) : folder(&f) {}
