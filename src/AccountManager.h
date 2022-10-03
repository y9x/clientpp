#pragma once
#include "./ClientFolder.h"
#include <map>
#include <string>
#include <rapidjson/fwd.h>

struct Account
{
  int order;
  std::string color;
  std::string password;
  rapidjson::Value dump(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  Account(rapidjson::Value &data);
  Account();
};

class AccountManager
{
private:
  ClientFolder &folder;
  std::wstring path = L"\\passwords.json";

public:
  // returns a base64 encoded array of bytes returned from CryptProtectData
  bool encrypt(std::string input, std::string &output);
  bool decrypt(std::string input, std::string &output);
  std::map<std::string, Account> data;
  bool save();
  bool load();
  std::string dump();
  AccountManager(ClientFolder &folder);
};