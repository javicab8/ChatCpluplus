#include "TCPServer.h"

TCPServer::TCPServer(int port, std::string databasePath)
{
    database.open(databasePath);
    serverPort = port;
	initializeWinsock();
	TCPServerAddress.sin_family = AF_INET;
	TCPServerAddress.sin_port = htons(serverPort);
	TCPServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	createSocket();
	showServerHostname();
	bindSocket();
	listenSocketConfiguration();
	FD_ZERO(&master);
	FD_SET(TCPServerSocket, &master);
}

TCPServer::~TCPServer()
{
	// Elimina el socket del servidor TCP de la lista maestra y lo cierra
	FD_CLR(TCPServerSocket, &master);
	closesocket(TCPServerSocket);

	std::string msg = "SERVER: El servidor se está apagando. Hasta luego\r\n";
	std::cout << msg;

	while (master.fd_count > 0)
	{
		// Obtiene el número de socket
		SOCKET sock = master.fd_array[0];

		// Envía el mensaje de despedida
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Elimina el socket cliente que ya recibió el mensaje de despedida
		FD_CLR(sock, &master);
		closesocket(sock);
		clients.erase(sock);
	}

	// Limpia winsock
	database.close();
	WSACleanup();
}

int TCPServer::initializeWinsock()
{
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		std::cerr << "No se pudo inicializar winsock! Finalizando...\r\n";
		return 1;
	}
    return 0;
}

void TCPServer::showServerHostname()
{
	char hostbuffer[256]{};
	gethostname(hostbuffer, sizeof(hostbuffer));

	std::cout << "Hostname: " << hostbuffer << "\r\n";
}

int TCPServer::createSocket()
{
	TCPServerSocket = socket(TCPServerAddress.sin_family, SOCK_STREAM, 0);
	if (TCPServerSocket == INVALID_SOCKET)
	{
		std::cerr << "No se pudo crear el socket! Finalizando...\r\n";
		return 1;
	}
    return 0;
}

int TCPServer::bindSocket()
{
	if (bind(TCPServerSocket, (sockaddr*)&TCPServerAddress, sizeof(TCPServerAddress)) != 0)
	{
		std::cerr << "No se pudo asociar la dirección ip con el socket! Finalizando...\r\n";
		return 1;
	}
    return 0;
}

int TCPServer::listenSocketConfiguration()
{
	if (listen(TCPServerSocket, SOMAXCONN) != 0)
	{
		std::cerr << "No se pudo configurar el socket para escuchar! Finalizando...\r\n";
		return 1;
	}
    return 0;
}

std::string TCPServer::getClientName()
{
	char hostname[NI_MAXHOST]{};
	char serverInfo[NI_MAXSERV]{};
	if (getnameinfo((sockaddr*)&clientAddress, sizeof(struct sockaddr), hostname, NI_MAXHOST, serverInfo, NI_MAXSERV, NI_NAMEREQD) != 0)
	{
		std::cerr << "Fallo obteniendo el nombre del dispositivo!\r\n";
		std::cerr << WSAGetLastError();
		return std::string();
	}
	return hostname;
}

void TCPServer::process()
{
	// Se hace una copia de la lista maestra para pasarla a la función select()
		// la función select() es destructiva
	fd_set copy = master;

	// Mira el socket que se está comunicando
	int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

	for (int i = 0; i < socketCount; i++)
	{
		// Se crea un socket auxiliar
		SOCKET sock = copy.fd_array[i];

		// Si es una comunicación entrante
		if (sock == TCPServerSocket)
		{
			// Acepta una nueva conexión
			int clientAddressLenght = sizeof(clientAddress);
			SOCKET client = accept(TCPServerSocket, (sockaddr*)&clientAddress, &clientAddressLenght);

			// Agrega el nuevo cliente a la lista maestra
			FD_SET(client, &master);
			clients[client] = getClientName();

			std::ostringstream welcomeMessageStream;
			welcomeMessageStream << "SERVER: Bienvenido! " << clients[client] << "\r\n";

			// Envía el mensaje de bienvenida al nuevo cliente
			std::string welcomeMessage = welcomeMessageStream.str();
			send(client, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);
		}
		else // Es un mensaje entrante
		{
			char buf[4096];
			ZeroMemory(buf, 4096);


			// Recibe mensaje
			int bytesIn = recv(sock, buf, 4096, 0);
			if (bytesIn <= 0)
			{
				// Saca el cliente de la lista maestra
				closesocket(sock);
				FD_CLR(sock, &master);
				clients.erase(sock);
			}
			else
			{
				std::ostringstream ss;
				std::string nowString = DateTime::getCurrentDateTimeString();
				ss << nowString << " " << clients[sock] << ": " << buf << "\r\n";

				database.insertRecord(nowString, clients[sock], buf);

				std::string strOut = ss.str();
				std::cout << strOut;

				// Envía el mensaje a los otros clientes
				for (u_int i = 0; i < master.fd_count; i++)
				{
					SOCKET outSock = master.fd_array[i];
					if (outSock != TCPServerSocket && outSock != sock)
					{
						send(outSock, strOut.c_str(), strOut.size() + 1, 0);
					}
				}

			}
		}
	}
}
