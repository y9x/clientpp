#pragma once
#include <iostream>
#include <ostream>
#include <streambuf>
#include <chrono>

class FileCout : private std::streambuf, public std::ostream {
private:
	std::string buffer;
	int overflow(int c) override;
public:
	std::wstring path;
	FileCout(std::wstring path);
};

extern FileCout fc;

std::string create_log_badge(std::string type);

#define LOG_INFO(data) fc << create_log_badge("Info") << data << '\n'
#define LOG_WARN(data) fc << create_log_badge("Warning") << data << '\n'
#define LOG_ERROR(data) fc << create_log_badge("Error") << data << '\n'