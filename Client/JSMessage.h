#pragma once
#include "./IPCMessages.h"
#include <json.hpp>
#include <wil/com.h>
#include <WebView2.h>

class JSMessage {
public:
	IM event = (IM)0;
	nlohmann::json args;
	JSMessage(nlohmann::json arguments);
	JSMessage(IM event);
	JSMessage(IM event, nlohmann::json args);
	JSMessage(LPWSTR raw);
	JSMessage();
	std::string dump();
	bool send(wil::com_ptr<ICoreWebView2> target);
	bool send(ICoreWebView2* target);
};