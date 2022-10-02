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
	std::wstring url;
	std::wstring bin = L"MicrosoftEdgeWebview2Setup.exe";
	std::wstring BinPath();
	WebView2Installer(std::wstring url);
	bool Installed();
	bool Install(Error& error);
};