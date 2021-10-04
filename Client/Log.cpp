#define _CRT_SECURE_NO_WARNINGS
#include "./Log.h"
#include "./Consts.h"

FileCout::FileCout(std::wstring p) : std::ostream(this), path(p) {}

int FileCout::overflow(int c) {
	if (c == EOF) return 0;

	buffer += c;

	if (c == '\n') {
#if WILL_LOG == 0
		FILE* handle = _wfopen(path.c_str(), L"a");
		if (handle) {
			fwrite(buffer.data(), sizeof(char), buffer.size(), handle);
			fclose(handle);
		}
#else
		std::cout << buffer;
#endif
		buffer.erase();
	}

	return 0;
}

FileCout fc(L"./TmpLogs.txt");

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