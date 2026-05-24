#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
using std::cerr;
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return result;
    }

    struct addrinfo* addr = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
    if (result != 0) {
        cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup();
        return 1;
    }

    int listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        cerr << "Error at socket: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }

    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cerr << "bind failed with error: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    cerr << "Сервер запущен на порту 8000" << endl;
    cerr << "Данные будут сохраняться в файл storage.txt" << endl;

    const int max_client_buffer_size = 1024;
    char buf[max_client_buffer_size];
    int client_socket = INVALID_SOCKET;

    for (;;) {
        client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << "\n";
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }

        result = recv(client_socket, buf, max_client_buffer_size, 0);
        std::stringstream response;
        std::stringstream response_body;

        if (result == SOCKET_ERROR) {
            cerr << "recv failed: " << result << "\n";
            closesocket(client_socket);
        }
        else if (result == 0) {
            cerr << "connection closed...\n";
        }
        else if (result > 0) {
            buf[result] = '\0';

            // ===== ДОБАВЛЕНО: СОХРАНЕНИЕ В ФАЙЛ =====
            if (strstr(buf, "POST") != NULL) {
                char* body = strstr(buf, "\r\n\r\n");
                if (body != NULL) {
                    body += 4;
                    FILE* file = fopen("storage.txt", "a");
                    if (file != NULL) {
                        fprintf(file, "%s\n", body);
                        fclose(file);
                        cerr << "Сохранено: " << body << endl;
                    }
                }
            }
            // ======================================

            // формируем тело ответа
            response_body << "<title>Test C++ HTTP Server</title>\n"
                << "<meta charset='UTF-8'>\n"  
                << "<h1>Test page</h1>\n"
                << "<p>This is body of the test page...</p>\n"
                << "<h2>Request headers</h2>\n"
                << "<pre>" << buf << "</pre>\n"
                << "<em><small>Test C++ Http Server</small></em>\n";

            response << "HTTP/1.1 200 OK\r\n"
                << "Version: HTTP/1.1\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << response_body.str().length()
                << "\r\n\r\n"
                << response_body.str();

            result = send(client_socket, response.str().c_str(), response.str().length(), 0);
            if (result == SOCKET_ERROR) {
                cerr << "send failed: " << WSAGetLastError() << "\n";
            }
            closesocket(client_socket);
        }
    }

    closesocket(listen_socket);
    freeaddrinfo(addr);
    WSACleanup();
    return 0;
}
