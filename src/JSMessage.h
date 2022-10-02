#pragma once
#include "./IPCMessages.h"
#include <WebView2.h>
#include <nlohmann/json.hpp>
#include <wil/com.h>

class JSMessage
{
public:
	int event = 0;
	nlohmann::json args;
	JSMessage(nlohmann::json arguments);
	JSMessage(int event);
	JSMessage(int event, nlohmann::json args);
	JSMessage(LPWSTR raw);
	JSMessage();
	std::string dump();
	bool send(wil::com_ptr<ICoreWebView2> target);
	bool send(ICoreWebView2 *target);
};