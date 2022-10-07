#include "./WebView2Installer.h"
#include "../utils/StringUtil.h"
#include "Log.h"
#include <WebView2EnvironmentOptions.h>
#include <net.h>

#define X64_webview                                                            \
  LR"(SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5})"
#define X86_webview                                                            \
  LR"(SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5})"

WebView2Installer::WebView2Installer(std::wstring u) : url(u) {}

std::wstring WebView2Installer::BinPath() {
  std::wstring path;
  path.resize(5000);
  path.resize(GetTempPath(path.size(), path.data()));
  path += L"\\" + bin;
  return path;
}

bool WebView2Installer::Install(Error &error) {
  error = Error::OK;

  std::wstring bin_path = BinPath();
  auto res = net::fetch_request(net::url(url));

  if (!res.size()) {
    error = Error::NoBytesDownloaded;
    return false;
  }

  HANDLE file = CreateFile(bin_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                           NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

  if (file != INVALID_HANDLE_VALUE) {
    DWORD bytes;
    WriteFile(file, res.data(), res.size(), &bytes, nullptr);
    CloseHandle(file);
    clog::info << "Downloaded " << ST::string(bin_path) << clog::endl;
  }

  STARTUPINFO startup;
  PROCESS_INFORMATION process;

  ZeroMemory(&startup, sizeof(startup));
  startup.cb = sizeof(process);
  ZeroMemory(&process, sizeof(process));

  if (CreateProcess(bin_path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL,
                    &startup, &process)) {
    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);

    return true;
  } else {
    error = Error::CantOpenProcess;
    return false;
  }
}

bool WebView2Installer::Installed() {
  wchar_t *version;
  HRESULT result =
      GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);

  if (!SUCCEEDED(result) && HRESULT_CODE(result) == ERROR_FILE_NOT_FOUND)
    return false;
  else
    return true;
}