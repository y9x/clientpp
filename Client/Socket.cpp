#include <WS2tcpip.h>
#include "./Socket.h"
#include "../Utils/StringUtil.h"
// #include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace StringUtil;

#define ERR_CASE(code) case code: return #code; break;
std::string last_err_str(int err) {
	switch (err) {
		ERR_CASE(WSANOTINITIALISED);
		ERR_CASE(WSAENETDOWN);
		ERR_CASE(WSAEINPROGRESS);
	default:
		return "0x" + std::to_string(err);
		break;
	}
}

std::string wsa_err_str(int err) {
	switch (err) {
	case SOCKET_ERROR:
		return last_err_str(WSAGetLastError());
		break;
	default:
		return "0x" + std::to_string(WSAGetLastError());
		break;
	}
}

Socket::Socket() {
	static bool init = false;

	if (!init) {
		WSADATA wsa_data;
		if (result = WSAStartup(MAKEWORD(winsock_min, 0), &wsa_data)) {
			throw 0;
		}
		else if (!LOBYTE(wsa_data.wVersion >= winsock_min)) {
			throw 1;
		}
		init = true;
	}

	socket = ::socket(family, type, protocol);

	if (socket == INVALID_SOCKET) {
		throw 2;
	}
}

void Socket::connect(std::string host, int port) {
	ADDRINFOW* addr_result = NULL;
	ADDRINFOW hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = type;
	hints.ai_protocol = protocol;

	if (result = GetAddrInfo(Convert::wstring(host).c_str(), std::to_wstring(port).c_str(), &hints, &addr_result)) {
		throw 3;
	}

	if (::connect(socket, addr_result->ai_addr, (int)addr_result->ai_addrlen) == SOCKET_ERROR) {
		throw 4;
	}

	FreeAddrInfo(addr_result);
}

void Socket::close() {
	if (socket != INVALID_SOCKET) ::closesocket(socket);
	socket = INVALID_SOCKET;
	WSACleanup();
}

Socket::~Socket() {
	close();
}

void Socket::send(std::string data) {
	::send(socket, data.c_str(), (int)data.size(), 0);
}

std::string Socket::read() {
	std::string buffer;
	buffer.resize(10000);
	buffer.resize(::recv(socket, &buffer[0], 1000, 0));
	return buffer;
}

void Socket::read_null() {
	::recv(socket, NULL, 0, 0);
}