#pragma once
#include <string>

// https://docs.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution

// Installs the WebView2 runtime
class WebView2Installer {
public:
	enum class Error {
		OK,
		CantOpenProcess,
		NoBytesDownloaded
	};
	std::string host, path;
	std::wstring bin = L"MicrosoftEdgeWebview2Setup.exe";
	std::wstring BinPath();
	WebView2Installer(std::string host, std::string path);
	bool Installed();
	bool Install(Error& error);
};