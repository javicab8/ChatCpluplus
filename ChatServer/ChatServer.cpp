#include "TCPServer.h"

int main()
{
	TCPServer tcpServer(54000, "chat.db");

	bool running = true;

	while (running)
	{
		tcpServer.process();
	}

	return 0;
}