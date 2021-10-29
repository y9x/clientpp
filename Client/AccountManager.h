#pragma once
#include "./ClientFolder.h"

class AccountManager {
private:
	ClientFolder* folder;
	std::wstring path = L"\\passwords.json";
	// JSON object organized by { username: encrypted password }
	nlohmann::json data;
	// returns a base64 encoded array of bytes returned from CryptProtectData
	bool encrypt(std::string input, std::string& output);
	bool decrypt(std::string input, std::string& output);
public:
	bool save();
	bool load();
	// override
	bool set(std::string name, std::string password);
	// reads an account password
	bool get(std::string name, std::string& password);
	// deletes an account
	bool remove(std::string name);
	std::vector<std::string> list();
	AccountManager(ClientFolder& folder);
};