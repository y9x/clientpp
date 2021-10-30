#pragma once
#include "./ClientFolder.h"

struct Account {
	int order;
	std::string color;
	std::string password;
	nlohmann::json dump();
	Account(nlohmann::json data);
};

class AccountManager {
private:
	ClientFolder* folder;
	std::wstring path = L"\\passwords.json";
	// returns a base64 encoded array of bytes returned from CryptProtectData
	bool encrypt(std::string input, std::string& output);
	bool decrypt(std::string input, std::string& output);
public:
	nlohmann::json dump();
	std::map<std::string, Account> data;
	bool save();
	bool load();
	// override
	bool set(std::string name, std::string password);
	// reads an account password
	bool get(std::string name, std::string& password);
	// deletes an account
	bool remove(std::string name);
	AccountManager(ClientFolder& folder);
};