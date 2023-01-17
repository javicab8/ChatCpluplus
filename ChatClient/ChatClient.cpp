#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

DWORD WINAPI clientReceive(LPVOID lpParam)
{
    char buffer[4096] = { 0 };

    // Crea el socket del servidor
    SOCKET server = *(SOCKET*)lpParam;

    while (true)
    {
        if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "No se pudo recibir datos, Error # " << WSAGetLastError() << "\r\n";
            return 1;
        }

        // Muestra el mensaje que llega del servidor
        std::cout << buffer;

        // Limpia la variable del mensaje
        memset(buffer, 0, sizeof(buffer));
    }
    return 0;
}

DWORD WINAPI clientSend(LPVOID lpParam)
{
    char buffer[4096] = { 0 };

    // Crea el socket del servidor
    SOCKET server = *(SOCKET*)lpParam;

    while (true) {

        // Captura el mensaje desde el teclado que se quiere enviar al servidor
        std::cin.getline(buffer, 4096);

        if (send(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) 
        {
            std::cerr << "send failed with error: " << WSAGetLastError() << "\r\n";
            return 1;
        }
    }
    return 0;
}

int main()
{
    std::string ipServerAddress = "127.0.0.1";
    int serverPort = 54000;

    // Iniciar winsock
    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0)
    {
        std::cerr << "No se pudo iniciar winsock, Error # " << wsResult << "\r\n";
        return 1;
    }

    // Crear socket
    SOCKET TCPClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (TCPClientSocket == INVALID_SOCKET)
    {
        std::cerr << "No se pudo crear el socket, Error # " << WSAGetLastError() << "\r\n";
        return 1;
    }

    // LLenar estructura de la direccion del servidor
    sockaddr_in TCPServerAddress{};
    TCPServerAddress.sin_family = AF_INET;
    TCPServerAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, ipServerAddress.c_str(), &TCPServerAddress.sin_addr);

    // Conectar al servidor
    int connectionResult = connect(TCPClientSocket, (sockaddr*)&TCPServerAddress, sizeof(TCPServerAddress));
    if (connectionResult == SOCKET_ERROR)
    {
        std::cerr << "No se pudo conectar al servidor, Error # " << WSAGetLastError() << "\r\n";
        return 1;
    }

    DWORD tid;

    // Crea hilo para recibir
    HANDLE receiveThread = CreateThread(nullptr, 0, clientReceive, &TCPClientSocket, 0, &tid);

    if (receiveThread == nullptr)
        std::cout << "Thread creation error: " << GetLastError();

    // Crea hilo para enviar
    HANDLE sendThread = CreateThread(nullptr, 0, clientSend, &TCPClientSocket, 0, &tid);

    if (sendThread == nullptr)
        std::cout << "Thread creation error: "<< GetLastError();

    WaitForSingleObject(receiveThread, INFINITE);
    WaitForSingleObject(sendThread, INFINITE);

    closesocket(TCPClientSocket);
    WSACleanup();
    return 0;
}
