//server
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")//подключаем библиотеку сокетов
#pragma warning(disable: 4996)//откл предупреждение об устаревших фкиях

using namespace std;

#define SRV_PORT 1234 //порт сервера
#define BUF_SIZE 256 //размер буфера для соо

const string QUEST = "Vvedite: 1(kamen), 2(nozhnicy), 3(bumaga) ili Bye dlya vyhoda\n";

int main() {
    setlocale(LC_ALL, "ru");
    srand(time(0)); //инициализация генератора случайных чисел
    char buff[1024]; // временный буфер для WSAStartup
    //инициализация  винсок
    if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }
    //создаем сокет
    SOCKET s, s_new; // s - слушающий сокет, s_new - для общения с клиентом
    int from_len; // длина адреса клиента
    char buf[BUF_SIZE] = { 0 }; // буфер приёма данных
    sockaddr_in sin, from_sin; // структуры адресов

    s = socket(AF_INET, SOCK_STREAM, 0); // создаём TCP сокет
    //настройка адреса сервера
    sin.sin_family = AF_INET; //IPv4
    sin.sin_addr.s_addr = 0; // 0 = любой сетевой интерфейс
    sin.sin_port = htons(SRV_PORT); // порт 1234 (htons переводит в сетевой порядок)

    bind(s, (sockaddr*)&sin, sizeof(sin)); //привязка сокета к порту
    listen(s, 3); //очередь до 3х клиентов

    string msg, msg1;// msg - отправляемое, msg1 - полученное

    while (1) {
        // ПРИНИМАЕМ ПОДКЛЮЧЕНИЕ КЛИЕНТА
        from_len = sizeof(from_sin);
        s_new = accept(s, (sockaddr*)&from_sin, &from_len);
        cout << "new connected client! \n";

        msg = QUEST; // первое сообщение - вопрос
        // ЦИКЛ ОБЩЕНИЯ С КЛИЕНТОМ 
        while (1) {
            // ОТПРАВЛЯЕМ СООБЩЕНИЕ КЛИЕНТУ
            send(s_new, (char*)&msg[0], msg.size(), 0);
            // ПОЛУЧАЕМ ОТВЕТ ОТ КЛИЕНТА
            from_len = recv(s_new, (char*)buf, BUF_SIZE, 0);
            buf[from_len] = 0; //ставим завершающий 0
            msg1 = (string)buf; // преобразуем в стринг
            cout << "Client: " << msg1 << endl;

            if (msg1 == "Bye") break;

            // Ход сервера (рандом)
            string server_choice;
            int r = rand() % 3;
            if (r == 0) server_choice = "1";
            else if (r == 1) server_choice = "2";
            else server_choice = "3";

            cout << "Server: " << server_choice << endl;

            // Определяем победителя
            string result;
            if (msg1 == server_choice) result = "НИЧЬЯ!";
            else if ((msg1 == "1" && server_choice == "2") ||
                (msg1 == "2" && server_choice == "3") ||
                (msg1 == "3" && server_choice == "1"))
                result = "KLIENT ПОБЕДИЛ!";
            else
                result = "SERVER ПОБЕДИЛ!";

            msg = result + "\n\n" + QUEST;// ФОРМИРУЕМ НОВОЕ СООБЩЕНИЕ (результат + вопрос)
        }

        cout << "client is lost" << endl;
        closesocket(s_new);//закрываем соединение с клиентом
    }

    return 0;
}
// client.cpp
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

#define SRV_HOST "localhost"
#define SRV_PORT 1234
#define CLNT_PORT 1235
#define BUF_SIZE 256

int main() {
    setlocale(LC_ALL, "ru");
    char buff[1024];

    if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }

    SOCKET s;
    int from_len;
    char buf[BUF_SIZE] = { 0 };
    hostent* hp; // структура для получения IP адреса
    sockaddr_in clnt_sin, srv_sin;

    s = socket(AF_INET, SOCK_STREAM, 0);

    clnt_sin.sin_family = AF_INET;
    clnt_sin.sin_addr.s_addr = 0;
    clnt_sin.sin_port = htons(CLNT_PORT); // порт 1235
    bind(s, (sockaddr*)&clnt_sin, sizeof(clnt_sin));     // ПРИВЯЗКА СОКЕТА К ПОРТУ КЛИЕНТА


    hp = gethostbyname(SRV_HOST); // ПОЛУЧАЕМ IP АДРЕС СЕРВЕРА ПО ИМЕНИ "localhost"
    // НАСТРОЙКА АДРЕСА СЕРВЕРА
    srv_sin.sin_port = htons(SRV_PORT);
    srv_sin.sin_family = AF_INET;
    // копируем IP адрес сервера из hp->h_addr_list
    ((unsigned long*)&srv_sin.sin_addr)[0] = ((unsigned long**)hp->h_addr_list)[0][0];
    // ПОДКЛЮЧАЕМСЯ К СЕРВЕРУ
    connect(s, (sockaddr*)&srv_sin, sizeof(srv_sin));

    string mst; // сообщение для отправки серверу


    do {
        // ПОЛУЧАЕМ СООБЩЕНИЕ ОТ СЕРВЕРА
        from_len = recv(s, (char*)&buf, BUF_SIZE, 0);
        if (from_len > 0) {
            buf[from_len] = 0;
            cout << buf << endl; // выводим сообщение (вопрос или результат)
        }
        // ЧИТАЕМ ВВОД ПОЛЬЗОВАТЕЛЯ
        getline(cin, mst);
        int msg_size = mst.size();
        send(s, (char*)&mst[0], msg_size, 0);// отправляем серверу

    } while (mst != "Bye");

    cout << "exit to infinity" << endl;
    cin.get();
    closesocket(s);// закрываем сокет

    return 0;
}
