#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.hpp>
#include "./WebView2Installer.h"
#include "../Utils/StringUtil.h"
#include "Log.h"
#include <WebView2EnvironmentOptions.h>

#define X64_webview LR"(SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5})"
#define X86_webview LR"(SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5})"

using namespace StringUtil;

WebView2Installer::WebView2Installer(std::string h, std::string p) : host(h), path(p) {}

std::wstring WebView2Installer::BinPath() {
	std::wstring path;
	path.resize(5000);
	path.resize(GetTempPath(path.size(), path.data()));
	path += L"\\" + bin;
	return path;
}

bool WebView2Installer::Install(Error& error) {
	error = Error::OK;
	
	std::wstring bin_path = BinPath();
	httplib::Client cli(host);
	cli.set_follow_location(true);
	auto res = cli.Get(path.c_str());
	
	if (!res->body.length()) {
		error = Error::NoBytesDownloaded;
		return false;
	}

	HANDLE file = CreateFile(bin_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE) {
		DWORD bytes;
		WriteFile(file, res->body.data(), res->body.length(), &bytes, nullptr);
		CloseHandle(file);
		clog::info << "Downloaded " << Convert::string(bin_path) << clog::endl;
	}

	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	
	ZeroMemory(&startup, sizeof(startup));
	startup.cb = sizeof(process);
	ZeroMemory(&process, sizeof(process));
	
	if (CreateProcess(bin_path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startup, &process)) {
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);

		return true;
	}
	else {
		error = Error::CantOpenProcess;
		return false;
	}
}

bool WebView2Installer::Installed() {
	wchar_t* version;
	HRESULT result = GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);

	if (!SUCCEEDED(result) && HRESULT_CODE(result) == ERROR_FILE_NOT_FOUND) return false;
	else return true;
}