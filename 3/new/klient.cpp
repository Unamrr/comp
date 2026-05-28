//клиент
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>//для сокетов
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")//для сокетов

using namespace std;

const char* SERVER = "json.org";//хотим вызвать этот сервер
const int PORT = 80;

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); //общение с сервером идет через сокет

    hostent* host = gethostbyname(SERVER);//спрашивает какой адрес  у джсон

    sockaddr_in addr = {};// Структура для хранения адреса
    addr.sin_family = AF_INET;// Семейство адресов (IPv4)
    addr.sin_port = htons(PORT);
    addr.sin_addr = *(in_addr*)host->h_addr;

    cout << "Connecting to " << SERVER << "..." << endl;
    connect(sock, (sockaddr*)&addr, sizeof(addr));// connect — устанавливает TCP-соединение с сервером
    cout << "Connected!" << endl;
    // Это текст, который мы отправим серверу. Он должен быть в точном формате HTTP
    string request =
        "GET / HTTP/1.1\r\n"
        "Host: " + string(SERVER) + "\r\n" //указываем какой сервер нам нужен
        "Connection: close\r\n"
        "\r\n";

    send(sock, request.c_str(), request.size(), 0);//отправляет даннные на сервер
    cout << "Request sent" << endl;

    char buffer[4096];
    int bytes;
    cout << "\n=== SERVER RESPONSE ===\n";
    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {   // recv — получает данные от сервера
        buffer[bytes] = '\0';
        cout << buffer;
    }
    cout << "\n=======================\n";

    closesocket(sock);
    WSACleanup();

    return 0;
}
