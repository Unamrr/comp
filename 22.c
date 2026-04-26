//klient
#define _WINSOCK_DEPRECATED_NO_WARNINGS   // отключаем предупреждения об устаревших функциях

#include <winsock2.h>         // работа с сокетами
#include <string>             
#include <windows.h>          // Windows API
#include <iostream>           

#pragma comment(lib, "ws2_32.lib")   // подключаем библиотеку Winsock
#pragma warning(disable: 4996)       // отключаем предупреждения

using namespace std;          

#define PORT 666              // порт сервера (должен совпадать с сервером)
#define SERVERADDR "127.0.0.1" // IP-адрес сервера (127.0.0.1 = localhost)

int main() {

    char buff[10 * 1014];     // буфер для данных (10140 байт)
    cout << "UDP Demo Client\nType quit to quit \n";

    //  ШАГ 1 - ИНИЦИАЛИЗАЦИЯ WINSOCK 
    if (WSAStartup(0x202, (WSADATA*)&buff)) {   // загружаем Winsock
        cout << "WSASTARTUP ERROR: " << WSAGetLastError() << "\n";
        return -1;            // ошибка - выходим
    }

    //  ШАГ 2 - СОЗДАНИЕ СОКЕТА 
    // Создаём UDP сокет (SOCK_DGRAM)
    SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (my_sock == INVALID_SOCKET) {   // проверка на ошибку
        cout << "SOCKET() ERROR: " << WSAGetLastError() << "\n";
        WSACleanup();        // выгружаем Winsock
        return -1;           // выходим
    }

    //  ШАГ 3 - НАСТРОЙКА АДРЕСА СЕРВЕРА 
    HOSTENT* hst;            // структура для имени хоста
    sockaddr_in Daddr;       // структура адреса сервера 
    Daddr.sin_family = AF_INET;        // IPv4
    Daddr.sin_port = htons(PORT);      // порт сервера (в сетевой порядок байт)

    // Определяем IP-адрес сервера (из имени или из строки)
    // inet_addr() - преобразует строку с IP (например "127.0.0.1") в число
    if (inet_addr(SERVERADDR)) {                     // если SERVERADDR - это IP-адрес
        Daddr.sin_addr.s_addr = inet_addr(SERVERADDR);  // используем его
    }
    else   // иначе пробуем получить IP по имени узла (например "localhost")
        if (hst = gethostbyname(SERVERADDR)) {       // получаем IP по имени
            Daddr.sin_addr.s_addr = ((unsigned long**)hst->h_addr_list)[0][0];  // берём первый IP
        }
        else {   // если ни то, ни другое не подошло
            cout << "Unknown Host: " << WSAGetLastError() << "\n";
            closesocket(my_sock);   // закрываем сокет
            WSACleanup();           // выгружаем Winsock
            return -1;              // выходим
        }

    //  ШАГ 4 - ОБМЕН СООБЩЕНИЯМИ С СЕРВЕРОМ 
    while (1) {                

        // Читаем сообщение с клавиатуры
        cout << "S<=C:";       // приглашение для ввода (S<=C = клиент отправляет)
        string SS;             // строка для введённого сообщения
        getline(cin, SS);      // читаем целую строку из консоли

        if (SS == "quit") break;   // если ввели "quit" - выходим из цикла

        // Отправляем датаграмму на сервер
        // sendto() - отправка UDP пакета
        // (char*)&SS[0] - указатель на данные (строку)
        // SS.size() - размер отправляемых данных
        // 0 - флаги
        // (sockaddr *)&Daddr - адрес получателя (сервера)
        // sizeof(Daddr) - размер структуры адреса
        sendto(my_sock, (char*)&SS[0], SS.size(), 0,
            (sockaddr*)&Daddr, sizeof(Daddr));

        // Принимаем ответ от сервера
        sockaddr_in SRaddr;    // структура для адреса отправителя (сервера)
        int SRaddr_size = sizeof(SRaddr);   // размер структуры

        // recvfrom() - принимаем UDP пакет
        int n = recvfrom(my_sock, buff, sizeof(buff), 0,
            (sockaddr*)&SRaddr, &SRaddr_size);

        if (n == SOCKET_ERROR) {   // если ошибка
            cout << "RECVFROM() ERROR:" << WSAGetLastError() << "\n";
            closesocket(my_sock);  // закрываем сокет
            WSACleanup();          // выгружаем Winsock
            return -1;             // выходим
        }

        buff[n] = '\0';        // добавляем завершающий ноль

        // Выводим полученный ответ на экран
        cout << "S=>C:" << buff << "\n";   // S=>C = сервер -> клиент
    }

    //  ШАГ 5 - ЗАВЕРШЕНИЕ РАБОТЫ 
    closesocket(my_sock);      // закрываем сокет
    WSACleanup();              // выгружаем библиотеку Winsock
    return 0;                  // успешное завершение
}


#define _WINSOCK_DEPRECATED_NO_WARNINGS   // отключаем предупреждения об устаревших функциях Winsock

#include <winsock2.h>         // заголовочный файл Winsock2 (работа с сокетами в Windows)
#include <windows.h>          // Windows API (нужен для Winsock)
#include <string>             
#include <iostream>          

#pragma comment(lib, "Ws2_32.lib")   // указываем линкеру подключить библиотеку Winsock
#pragma warning(disable: 4996)       // отключаем предупреждение 4996 (опасные функции)

using namespace std;          

#define PORT 666              // порт, на котором будет работать сервер
#define sHELLO "Hello, STUDENT\n"   

int main() {                  

    char buff[1024];          // буфер для хранения данных (1024 байта)
    cout << "UDP DEMO ECHO-SERVER\n";   // выводим название программы

    //  ШАГ 1 - ИНИЦИАЛИЗАЦИЯ WINSOCK 
    // WSAStartup - загружает библиотеку Winsock
    // 0x202 - версия Winsock 2.2
    // (WSADATA *)&buff[0] - структура для получения информации о версии
    if (WSAStartup(0x202, (WSADATA*)&buff[0])) {  // если не 0 - ошибка
        cout << "WSAStartup error: " << WSAGetLastError();  // выводим код ошибки
        return -1;           // завершаем программу с кодом ошибки
    }

    //  ШАГ 2 - СОЗДАНИЕ СОКЕТА 
    SOCKET Lsock;             // дескриптор сокета (аналог файлового дескриптора)
    // socket() - создает сокет
    // AF_INET - семейство протоколов IPv4
    // SOCK_DGRAM - тип сокета: датаграммный (UDP)
    // 0 - протокол (0 = автоматически выбирает UDP для SOCK_DGRAM)
    Lsock = socket(AF_INET, SOCK_DGRAM, 0);

    if (Lsock == INVALID_SOCKET) {     // если сокет не создан (ошибка)
        cout << "SOCKET() ERROR: " << WSAGetLastError();  // выводим ошибку
        WSACleanup();        // выгружаем библиотеку Winsock
        return -1;           // завершаем программу
    }

    //  ШАГ 3 - СВЯЗЫВАНИЕ СОКЕТА С ЛОКАЛЬНЫМ АДРЕСОМ 
    sockaddr_in Laddr;       // структура для хранения адреса сервера
    Laddr.sin_family = AF_INET;        // семейство протоколов: IPv4
    Laddr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0 - принимать соединения с любых IP-адресов
    Laddr.sin_port = htons(PORT);       // htons() - переводит порт из host byte order в network byte order

    // bind() - привязывает сокет к указанному адресу и порту
    // (sockaddr *)&Laddr - приводим к общему типу sockaddr
    // sizeof(Laddr) - размер структуры
    if (bind(Lsock, (sockaddr*)&Laddr, sizeof(Laddr))) {  // если не 0 - ошибка
        cout << "BIND ERROR:" << WSAGetLastError();        // выводим ошибку
        closesocket(Lsock);   // закрываем сокет
        WSACleanup();         // выгружаем Winsock
        return -1;            // завершаем программу
    }

    //  ШАГ 4 - ОБРАБОТКА ДАТАГРАММ ОТ КЛИЕНТОВ 
    while (1) {               // бесконечный цикл (сервер работает постоянно)

        sockaddr_in Caddr;    // структура для хранения адреса клиента
        int Caddr_size = sizeof(Caddr);   // размер структуры (нужен для recvfrom)

        // recvfrom() - принимает данные от клиента (UDP)
        // &buff[0] - буфер для данных
        // sizeof(buff)-1 - максимальный размер данных (оставляем место для '\0')
        // 0 - флаги (0 = нет спец.режимов)
        // (sockaddr *)&Caddr - адрес клиента будет записан сюда
        // &Caddr_size - размер структуры (in/out параметр)
        int bsize = recvfrom(Lsock, &buff[0], sizeof(buff) - 1, 0,
            (sockaddr*)&Caddr, &Caddr_size);

        if (bsize == SOCKET_ERROR) {     // если ошибка
            cout << "RECVFROM() ERROR:" << WSAGetLastError();  // выводим код ошибки
        }

        //  ОПРЕДЕЛЯЕМ IP-АДРЕС КЛИЕНТА 
        HOSTENT* hst;         // структура для информации об узле (IP -> имя)
        // gethostbyaddr() - получает имя хоста по IP-адресу
        // (char *)&Caddr.sin_addr - указатель на IP адрес клиента
        // 4 - длина IP-адреса (4 байта для IPv4)
        // AF_INET - семейство протоколов
        hst = gethostbyaddr((char*)&Caddr.sin_addr, 4, AF_INET);

        // Выводим информацию о клиенте
        cout << "NEW DATAGRAM!\n" <<
            ((hst) ? hst->h_name : "Unknown host") << "/n" <<   // имя хоста или "Unknown host"
            inet_ntoa(Caddr.sin_addr) << "/n" <<                 // IP-адрес в виде строки
            ntohs(Caddr.sin_port) << '\n';                       // порт клиента (ntohs - обратно в host byte order)

        buff[bsize] = '\0';    // добавляем завершающий ноль (чтобы работать как со строкой)
        cout << "C=>S:" << buff << '\n';   // выводим полученное сообщение на экран

        //  ОТПРАВЛЯЕМ ДАННЫЕ ОБРАТНО КЛИЕНТУ (ЭХО) 
        // sendto() - отправляет данные клиенту
        // &buff[0] - данные для отправки
        // bsize - размер отправляемых данных
        // 0 - флаги
        // (sockaddr *)&Caddr - адрес получателя (клиента)
        // sizeof(Caddr) - размер структуры адреса
        sendto(Lsock, &buff[0], bsize, 0, (sockaddr*)&Caddr, sizeof(Caddr));
    }

    return 0;                // завершаем программу (никогда не выполняется из-за while(1))
}
