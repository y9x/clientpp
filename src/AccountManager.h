#pragma once
#include "./ClientFolder.h"
#include <map>
#include <string>
#include <rapidjson/fwd.h>

struct Account
{
  int order = -1;
  std::string username;
  std::string password;
  std::string color;
  rapidjson::Value dump(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  Account(const rapidjson::Value &data);
  Account();
};

class AccountManager
{
private:
  ClientFolder &folder;
  std::wstring path = L"\\passwords.json";
  std::string dump();

public:
  // returns a base64 encoded array of bytes returned from CryptProtectData
  bool encrypt(std::string input, std::string &output);
  bool decrypt(std::string input, std::string &output);
  std::map<std::string, Account> data;
  bool save();
  bool load();
  rapidjson::Value dump(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);
  AccountManager(ClientFolder &folder);
};