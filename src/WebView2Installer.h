#pragma once
#include <string>

// https://docs.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution

// Installs the WebView2 runtime
class WebView2Installer {
public:
  enum class Error { OK, CantOpenProcess, NoBytesDownloaded };
  std::wstring binPath;
  /// @return if WebView2 is present on the machine
  static bool isInstalled();
  /// @brief Install the runtime.
  bool install(Error &error);
  WebView2Installer();
};