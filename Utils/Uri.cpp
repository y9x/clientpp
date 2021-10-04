#include "./Uri.h"

Uri::Uri(std::wstring u) : uri(u) {}

std::wstring Uri::protocol() {
	size_t first = uri.find_first_of(L':');
	if (first != std::wstring::npos)return uri.substr(0, first + 1);
	else return L"";
}

std::wstring Uri::host(bool remove_www) {
	std::wstring host = uri.substr(protocol().length());

	if (host.starts_with(L"//")) host = host.substr(2);

	if (remove_www && host.starts_with(L"www.")) host = host.substr(4);

	size_t first_slash = host.find_first_of(L'/');
	if (first_slash != std::wstring::npos) host = host.substr(0, first_slash);

	return host;
}

std::wstring Uri::origin() {
	return protocol() + L"//" + host();
}

std::wstring Uri::path() {
	return uri.substr(origin().length());
}

std::wstring Uri::search() {
	std::wstring p = path();
	size_t first = p.find_first_of(L'?');
	if (first != std::wstring::npos)return p.substr(first);
	else return L"";
}

std::wstring Uri::pathname() {
	std::wstring p = path();
	return p.substr(0, p.length() - search().length());
}

bool Uri::HostEquals(std::wstring match) {
	std::wstring h = host();

	return h == match || h.ends_with(L'.' + match);
}