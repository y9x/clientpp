#define _CRT_SECURE_NO_WARNINGS
#include "./Log.h"
#include <Windows.h>

namespace clog {
	char endl = '\n';
	std::wstring logs;

#if _DEBUG == 1
	constexpr bool cout = true;
#else
	constexpr bool cout = false;
#endif

	int FileOut::overflow(int c) {
		if (c == EOF) return 0;

		buffer += c;

		if (c == endl) {
			std::string prefix;

			if (cout || badge_file) prefix += "[" + badge + "] ";

			if (!cout) {
				std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				std::string ts = std::ctime(&now);
				ts.pop_back();
				prefix += "[" + ts + "] ";
			}

			prefix.pop_back();
			
			buffer = prefix + ": " + buffer;

			if(cout){
				std::cout << buffer;
			}
			else {
				FILE* handle = _wfopen((logs + file).c_str(), L"a");
				if (handle) {
					fwrite(buffer.data(), sizeof(char), buffer.size(), handle);
					fclose(handle);
				}
			}

			buffer.erase();
		}

		return 0;
	}

	bool console_attached = false;

	bool attach_console() {
		if (console_attached) return true;

#if _DEBUG == 1
		SetConsoleCtrlHandler(0, true);
		if (AllocConsole()) {
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			freopen("CONIN$", "r", stdin);

			std::cout.clear();
			std::cin.clear();
		}
		else {
			MessageBox(NULL, L"Failure attatching console", L"Debugger", MB_OK);
			return false;
		}
#endif
		return true;
	}

	FileOut::FileOut(std::string b, std::wstring f, bool c, bool bf) : badge(b), badge_file(bf), file(f), std::ostream(this) {
		console_attached = attach_console();
	}

	// badges for shared log files
	FileOut info("Info", L"\\Info.log", true);
	FileOut warn("Warning", L"\\Info.log", true);
	FileOut error("Error", L"\\Error.log");
	FileOut debug("Debug", L"\\Debug.log");
};