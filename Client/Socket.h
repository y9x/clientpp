#pragma once
#include <Windows.h>
#include <string>

class Socket {
private:
	SOCKET socket;
	DWORD result = 0;
	int winsock_min = 2;
	int family = AF_INET;
	int type = SOCK_STREAM;
	int protocol = IPPROTO_TCP;
public:
	Socket();
	~Socket();
	void connect(std::string host, int port);
	void close();
	void send(std::string data);
	std::string read();
	void read_null();
};