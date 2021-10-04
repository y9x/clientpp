#pragma once
#include <ostream>
#include <streambuf>
#include <chrono>
#include "./Consts.h"

#if WILL_LOG == 1
#include <iostream>
#define LOG_COUT std::cout
#else
class FileCout : private std::streambuf, public std::ostream {
private:
	std::string buffer;
	int overflow(int c) override;
public:
	std::wstring path;
	FileCout(std::wstring path);
};

extern FileCout fc;
#define LOG_COUT fc
#endif

std::string create_log_badge(std::string type);

#define LOG_INFO(data) LOG_COUT << create_log_badge("Info") << data << '\n'
#define LOG_WARN(data) LOG_COUT << create_log_badge("Warning") << data << '\n'
#define LOG_ERROR(data) LOG_COUT << create_log_badge("Error") << data << '\n'