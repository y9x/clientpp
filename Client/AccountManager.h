#pragma once
#include "./ClientFolder.h"

struct Account {
	int order;
	std::string color;
	std::string password;
	nlohmann::json dump();
	Account(nlohmann::json data);
	Account();
};

class AccountManager {
private:
	ClientFolder* folder;
	std::wstring path = L"\\passwords.json";
public:
	// returns a base64 encoded array of bytes returned from CryptProtectData
	bool encrypt(std::string input, std::string& output);
	bool decrypt(std::string input, std::string& output);
	nlohmann::json dump();
	std::map<std::string, Account> data;
	bool save();
	bool load();
	AccountManager(ClientFolder& folder);
};