#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <map>

#include "DateTime.h"
#include "SQLiteChatDB.h"

#pragma once
#pragma comment (lib, "ws2_32.lib")
class TCPServer
{
public:
	TCPServer(int port, std::string databasePath);
	~TCPServer();
	int initializeWinsock();
	void showServerHostname();
	int createSocket();
	int bindSocket();
	int listenSocketConfiguration();
	std::string getClientName();
	void process();

private:
	SQLiteChatDB database;
	int serverPort;
	sockaddr_in TCPServerAddress;
	SOCKET TCPServerSocket;
	sockaddr clientAddress;
	std::map<int, std::string> clients;
	fd_set master;
};

