#pragma once
#include <string>
#include <WebView2.h>
#include <rapidjson/document.h>
#include <rapidjson/fwd.h>
#include <wil/com.h>
#include "./IPCMessages.h"

class JSMessage
{
public:
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator;
	char event;
	// std::vector<rapidjson::Value> args;
	rapidjson::Value args;
	JSMessage(const JSMessage &message);
	JSMessage(unsigned char event);
	JSMessage(unsigned char event, const rapidjson::Value &args);
	JSMessage(LPWSTR raw);
	JSMessage();
	std::string dump();
	bool send(wil::com_ptr<ICoreWebView2> target);
	bool send(ICoreWebView2 *target);
};