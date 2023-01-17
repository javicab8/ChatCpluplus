#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <map>

#include "DateTime.h"
#include "SQLiteChatDB.h"

#pragma comment (lib, "ws2_32.lib")

int main()
{
	SQLiteChatDB database;
	database.open("chat.db");
	
	// Inicializa winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		std::cerr << "No se pudo inicializar winsock! Finalizando...\r\n";
		return 0;
	}
	char hostbuffer[256]{};
	gethostname(hostbuffer, sizeof(hostbuffer));

	std::cout << "Hostname: " << hostbuffer << "\r\n";

	// Enlaza la dirección ip y el puerto con el socket del servidor
	sockaddr_in TCPServerAddress{};
	TCPServerAddress.sin_family = AF_INET;
	TCPServerAddress.sin_port = htons(54000);
	TCPServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;

	char hostname[NI_MAXHOST]{};
	char serverInfo[NI_MAXSERV]{};

	sockaddr clientAddress{};
	int clientAddressLenght = sizeof(clientAddress);

	std::map<int, std::string> clients;

	// Crea un socket
	SOCKET TCPServerSocket = socket(TCPServerAddress.sin_family, SOCK_STREAM, 0);
	if (TCPServerSocket == INVALID_SOCKET)
	{
		std::cerr << "No se pudo crear el socket! Finalizando...\r\n";
		return 0;
	}

	// Asocia la dirección ip con el socket
	if (bind(TCPServerSocket, (sockaddr*)&TCPServerAddress, sizeof(TCPServerAddress)) != 0)
	{
		std::cerr << "No se pudo asociar la dirección ip con el socket! Finalizando...\r\n";
		return 0;
	}

	// Configura el estado del socket para escuchar
	if (listen(TCPServerSocket, SOMAXCONN) != 0)
	{
		std::cerr << "No se pudo configurar el socket para escuchar! Finalizando...\r\n";
		return 0;
	}

	// Crea la lista maestra y la establece a cero
	fd_set master{};
	FD_ZERO(&master);

	// Se agrega a la lista maesta el socket del servidor como socket inicial que permite escuchar  
	FD_SET(TCPServerSocket, &master);

	bool running = true;

	while (running)
	{
		// Se hace una copia de la lista maestra para pasarla a la función select()
		// la función select() es destructiva
		fd_set copy = master;

		// Mira el socket que se está comunicando
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Recorre las conexiones actuales y posibles conexiones nuevas
		for (int i = 0; i < socketCount; i++)
		{
			// Se crea un socket auxiliar
			SOCKET sock = copy.fd_array[i];

			// Si es una comunicación entrante
			if (sock == TCPServerSocket)
			{
				// Acepta una nueva conexión
				SOCKET client = accept(TCPServerSocket, (sockaddr *)&clientAddress, &clientAddressLenght);

				// Agrega el nuevo cliente a la lista maestra
				FD_SET(client, &master);
				
				if (getnameinfo((sockaddr*)&clientAddress, sizeof(struct sockaddr), hostname, NI_MAXHOST, serverInfo, NI_MAXSERV, NI_NAMEREQD) != 0)
				{
					std::cerr << "Falló obteniendo el nombre del dispositivo! Finalizando...\r\n";
					std::cerr << WSAGetLastError();
					return 0;
				}

				clients[client] = hostname;
				std::ostringstream welcomeMessageStream;
				welcomeMessageStream << "SERVER: Bienvenido! " << hostname << "\r\n";
				
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
	WSACleanup();
	return 1;
}
