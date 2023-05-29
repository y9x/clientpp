#define _CRT_SECURE_NO_WARNINGS
#include "./Log.h"
#include <Windows.h>
#include <chrono>

namespace clog {
char endl = '\n';
std::wstring logs;

#if _DEBUG == 1
constexpr bool vdebug = true;
#else
constexpr bool vdebug = false;
#endif

int FileOut::overflow(int c) {
  if (!work || c == EOF)
    return 0;

  buffer += c;

  if (c == endl) {
    std::string prefix;

    if (vdebug || badgeFile)
      prefix += "[" + badge + "] ";

    if (!vdebug) {
      std::time_t now = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
      std::string ts = std::ctime(&now);
      ts.pop_back();
      prefix += "[" + ts + "] ";
    }

    prefix.pop_back();

    buffer = prefix + ": " + buffer;

    if (vdebug) {
      std::cout << buffer;
    } else {
      FILE *handle = _wfopen((logs + file).c_str(), L"a");
      if (handle) {
        fwrite(buffer.data(), sizeof(char), buffer.size(), handle);
        fclose(handle);
      }
    }

    buffer.erase();
  }

  return 0;
}

bool consoleAttached = false;

bool attachConsole() {
  if (consoleAttached)
    return true;

#if _DEBUG == 1
  SetConsoleCtrlHandler(0, true);
  if (AllocConsole()) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    std::cout.clear();
    std::cin.clear();
  } else {
    MessageBox(NULL, L"Failure attatching console", L"Debugger", MB_OK);
    return false;
  }
#endif
  return true;
}

FileOut::FileOut(std::string b, std::wstring f, bool bf, bool work)
    : work(work), badge(b), badgeFile(bf), file(f), std::ostream(this) {
  consoleAttached = attachConsole();
}

// badges for shared log files
FileOut info("Info", L"\\info.log", true);
FileOut warn("Warning", L"\\info.log", true);
FileOut error("Error", L"\\error.log");
FileOut debug("Debug", L"\\debug.log", false, vdebug);
}; // namespace clog