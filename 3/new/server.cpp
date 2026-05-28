//сервер

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int PORT = 8888;

// HTML-страница, которую сервер будет отдавать всем клиентам
const string HTML_PAGE =
"<html>"
"<head>"
"<title>HTTP Server</title>"
"</head>"
"<body>"
"<h1>Server is running!</h1>"
"<p>This is HTTP response from my server</p>"
"<p>Lab 3: HTTP Connection</p>"
"</body>"
"</html>";
// Функция для сборки HTTP-ответа
// Берёт тело страницы и добавляет к нему заголовки
string make_response(const string& body) {
    ostringstream resp;
    resp << "HTTP/1.1 200 OK\r\n"//статус все хорошо
        << "Content-Type: text/html\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    return resp.str();
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0.0.0.0 — значит, что сервер будет доступен и с localhost,
    // и с других компьютеров в сети

    bind(server, (sockaddr*)&addr, sizeof(addr));//говорит системе на каком порту будет работать сокет
    listen(server, 10); // listen — переводим сокет в режим ожидания подключений

    cout << "Server started on http://localhost:" << PORT << endl;

    while (true) {
        SOCKET client = accept(server, NULL, NULL); // Когда клиент подключается, accept возвращает НОВЫЙ сокет для общения с ним

        char buffer[1024];
        recv(client, buffer, sizeof(buffer), 0);

        string response = make_response(HTML_PAGE);//формируем и отправляем ответ
        send(client, response.c_str(), response.size(), 0);

        closesocket(client);
        // Возвращаемся к accept() — ждать следующего клиента
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
