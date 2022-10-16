#include "./WebView2Installer.h"
#include "../utils/StringUtil.h"
#include "./fetch.h"
#include "Log.h"
#include <WebView2EnvironmentOptions.h>

#define X64_webview                                                            \
  LR"(SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5})"
#define X86_webview                                                            \
  LR"(SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5})"

WebView2Installer::WebView2Installer() {
  binPath.resize(MAX_PATH);
  binPath.resize(GetTempPath(binPath.size(), binPath.data()));
  binPath += L"\\MicrosoftEdgeWebview2Setup";
}

bool WebView2Installer::install(Error &error) {
  error = Error::OK;

  auto res = fetchGet("https://go.microsoft.com/fwlink/p/?LinkId=2124703");

  if (!res.size()) {
    error = Error::NoBytesDownloaded;
    return false;
  }

  HANDLE file = CreateFile(binPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                           NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

  if (file != INVALID_HANDLE_VALUE) {
    DWORD bytes;
    WriteFile(file, res.data(), res.size(), &bytes, nullptr);
    CloseHandle(file);
    clog::info << "Downloaded " << ST::string(binPath) << clog::endl;
  }

  STARTUPINFO startup;
  PROCESS_INFORMATION process;

  ZeroMemory(&startup, sizeof(startup));
  startup.cb = sizeof(process);
  ZeroMemory(&process, sizeof(process));

  if (CreateProcess(binPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL,
                    &startup, &process)) {
    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);

    return true;
  } else {
    error = Error::CantOpenProcess;
    return false;
  }
}

bool WebView2Installer::isInstalled() {
  wchar_t *version;
  HRESULT result =
      GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);

  return SUCCEEDED(result);
  // || HRESULT_CODE(result) != ERROR_FILE_NOT_FOUND;
}