#pragma once

#if WILL_LOG == 1
#define LOG_COUT std::cout
#else
class FileCout : private std::streambuf, public std::ostream {
private:
	std::string buffer;
	int overflow(int c) override {
		if (c == EOF) return 0;

		buffer += c;

		if (c == '\n') {
			FILE* handle = _wfopen(path.c_str(), L"a");
			if (handle) {
				fwrite(buffer.data(), sizeof(char), buffer.size(), handle);
				fclose(handle);
			}
			buffer.erase();
		}

		return 0;
	}
public:
	std::wstring path;
	FileCout(std::wstring p) : std::ostream(this), path(p) {}
};

FileCout fc(L"./TmpLogs.txt");
#define LOG_COUT fc
#endif

std::string create_log_badge(std::string type) {
	std::string result = "[" + type + "]";

#if WILL_LOG == 0
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string ts = std::ctime(&now);
	ts.pop_back();
	result += " [" + ts + "]";
#endif

	result += ": ";

	return result;
}

#define LOG_INFO(data) LOG_COUT << create_log_badge("Info") << data << '\n'
#define LOG_WARN(data) LOG_COUT << create_log_badge("Warning") << data << '\n'
#define LOG_ERROR(data) LOG_COUT << create_log_badge("Error") << data << '\n'