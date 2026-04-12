#include "stdafx.h"  // Предкомпилированный заголовок Visual Studio

#define _WINSOCK_DEPRECATED_NO_WARNINGS  // Отключаем предупреждения об устаревших функциях

#include <winsock2.h>  // Основная библиотека сокетов Windows
#include <windows.h>   // Windows-специфичные функции
#include <string>      // Для работы со строками
#include <iostream>    // Для cout, cin

#pragma comment(lib, "Ws2_32.lib")  // Линкуем библиотеку сокетов
#pragma warning(disable: 4996)      // Отключаем предупреждение 4996

using namespace std;  // Чтобы не писать std::

#define PORT 666      // Номер порта сервера (используем 666)
#define sHELLO "Hello, STUDENT\n"  // Приветственное сообщение (не используется в коде)

int main() {
    
    char buff[1024];  // Буфер для хранения данных (1024 байта)
    
    cout << "UDP DEMO ECHO-SERVER\n";  // Выводим название программы
    
    // ========== ШАГ 1 - ПОДКЛЮЧЕНИЕ БИБЛИОТЕКИ Winsock ==========
    // WSAStartup - инициализация WinSock
    // 0x202 - версия WinSock 2.2 (0x202 = 2.2 в шестнадцатеричной)
    if (WSAStartup(0x202, (WSADATA*)&buff[0])) {
        // Если не 0 - ошибка
        cout << "WSAStartup error: " << WSAGetLastError();
        return -1;
    }
    
    // ========== ШАГ 2 - СОЗДАНИЕ СОКЕТА ==========
    SOCKET Lsock;  // Дескриптор сокета сервера
    
    // socket(AF_INET, SOCK_DGRAM, 0)
    // AF_INET - семейство протоколов IPv4
    // SOCK_DGRAM - тип сокета для UDP (датаграммный)
    // 0 - протокол по умолчанию (UDP)
    Lsock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (Lsock == INVALID_SOCKET) {  // Проверка на ошибку
        cout << "SOCKET() ERROR: " << WSAGetLastError();
        WSACleanup();  // Очищаем ресурсы WinSock
        return -1;
    }
    
    // ========== ШАГ 3 - СВЯЗЫВАНИЕ СОКЕТА С ЛОКАЛЬНЫМ АДРЕСОМ ==========
    sockaddr_in Laddr;  // Структура для адреса сервера
    
    Laddr.sin_family = AF_INET;           // IPv4
    Laddr.sin_addr.s_addr = INADDR_ANY;   // INADDR_ANY = 0 (слушаем все интерфейсы)
    Laddr.sin_port = htons(PORT);         // Порт 666 (преобразуем в сетевой порядок байт)
    
    // bind() - привязывает сокет к порту
    if (bind(Lsock, (sockaddr*)&Laddr, sizeof(Laddr))) {
        cout << "BIND ERROR:" << WSAGetLastError();
        closesocket(Lsock);  // Закрываем сокет
        WSACleanup();        // Очищаем WinSock
        return -1;
    }
    
    // ========== ШАГ 4 - ОБРАБОТКА ПАКЕТОВ ОТ КЛИЕНТОВ ==========
    while (1) {  // Бесконечный цикл (сервер работает постоянно)
        
        sockaddr_in Caddr;          // Структура для адреса клиента
        int Caddr_size = sizeof(Caddr);  // Размер структуры адреса
        
        // recvfrom() - ПРИНИМАЕТ ДАТАГРАММУ от клиента
        // Параметры:
        // Lsock - наш сокет
        // &buff[0] - буфер для данных
        // sizeof(buff)-1 - максимальный размер данных
        // 0 - флаги
        // (sockaddr*)&Caddr - адрес отправителя (заполнится автоматически)
        // &Caddr_size - размер адреса отправителя
        int bsize = recvfrom(Lsock, &buff[0], sizeof(buff)-1, 0,
                             (sockaddr*)&Caddr, &Caddr_size);
        
        if (bsize == SOCKET_ERROR) {  // Проверка на ошибку приёма
            cout << "RECVFROM() ERROR:" << WSAGetLastError();
        }
        
        // ========== ОПРЕДЕЛЯЕМ IP-АДРЕС КЛИЕНТА ==========
        HOSTENT* hst;  // Структура для информации о хосте
        
        // gethostbyaddr() - преобразует IP-адрес в имя хоста
        // (char*)&Caddr.sin_addr - указатель на IP-адрес
        // 4 - длина адреса (IPv4 = 4 байта)
        // AF_INET - семейство адресов
        hst = gethostbyaddr((char*)&Caddr.sin_addr, 4, AF_INET);
        
        // Выводим информацию о клиенте
        cout << "NEW DATAGRAM!\n";
        // Если hst не NULL - выводим имя хоста, иначе "Unknown host"
        cout << ((hst) ? hst->h_name : "Unknown host") << "\n";
        // inet_ntoa() - преобразует IP в строку (например "192.168.1.1")
        cout << inet_ntoa(Caddr.sin_addr) << "\n";
        // ntohs() - преобразует порт из сетевого порядка в обычный
        cout << ntohs(Caddr.sin_port) << '\n';
        
        buff[bsize] = '\0';  // Добавляем завершающий нуль (для вывода как строки)
        cout << "C=>S:" << buff << '\n';  // Выводим полученное сообщение
        
        // ========== ОТПРАВЛЯЕМ ДАННЫЕ ОБРАТНО КЛИЕНТУ (ЭХО) ==========
        // sendto() - ОТПРАВЛЯЕТ ДАТАГРАММУ клиенту
        // Lsock - наш сокет
        // &buff[0] - данные для отправки
        // bsize - количество байт (столько же, сколько приняли)
        // 0 - флаги
        // (sockaddr*)&Caddr - адрес получателя (клиента)
        // sizeof(Caddr) - размер адреса
        sendto(Lsock, &buff[0], bsize, 0, (sockaddr*)&Caddr, sizeof(Caddr));
        
    }  // Конец while - возвращаемся к приёму следующей датаграммы
    
    return 0;  // Сюда никогда не дойдём из-за бесконечного цикла
}

#define _WINSOCK_DEPRECATED_NO_WARNINGS  // Отключаем предупреждения

#include <winsock2.h>   // Основная библиотека сокетов
// #include "stdafx.h"  // Закомментировано (не нужно)
#include <string>       // Для работы со строками
#include <windows.h>    // Windows-функции
#include <iostream>     // Для cout, cin, getline

#pragma comment(lib, "ws2_32.lib")  // Линкуем библиотеку сокетов
#pragma warning(disable: 4996)      // Отключаем предупреждение 4996

using namespace std;  // Пространство имён std

#define PORT 666               // Порт сервера (должен совпадать с сервером)
#define SERVERADDR "127.0.0.1" // IP-адрес сервера (localhost)

int main() {
    
    char buff[10 * 1014];  // Буфер размером 10140 байт (чуть больше 10 КБ)
    
    cout << "UDP Demo Client\nType quit to quit \n";  // Приветствие
    
    // ========== ШАГ 1 - ИНИЦИАЛИЗАЦИЯ BIBLIOTEKI WINSOCK ==========
    if (WSAStartup(0x202, (WSADATA*)&buff)) {
        cout << "WSASTARTUP ERROR: " << WSAGetLastError() << "\n";
        return -1;
    }
    
    // ========== ШАГ 2 - ОТКРЫТИЕ СОКЕТА ==========
    // socket(AF_INET, SOCK_DGRAM, 0) - создаём UDP-сокет
    SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (my_sock == INVALID_SOCKET) {  // Проверка на ошибку
        cout << "SOCKET() ERROR: " << WSAGetLastError() << "\n";
        WSACleanup();
        return -1;
    }
    
    // ========== ШАГ 3 - НАСТРОЙКА АДРЕСА СЕРВЕРА ==========
    HOSTENT* hst;             // Для преобразования имени хоста в IP
    sockaddr_in Daddr;        // Адрес сервера (Destination address)
    
    Daddr.sin_family = AF_INET;           // IPv4
    Daddr.sin_port = htons(PORT);         // Порт 666 в сетевом порядке
    
    // ОПРЕДЕЛЕНИЕ IP-АДРЕСА СЕРВЕРА
    // inet_addr() - преобразует строку "127.0.0.1" в 32-битный IP-адрес
    if (inet_addr(SERVERADDR)) {
        // Если строка - это уже IP-адрес (например "127.0.0.1")
        Daddr.sin_addr.s_addr = inet_addr(SERVERADDR);
    }
    else if (hst = gethostbyname(SERVERADDR)) {
        // Если это имя хоста (например "localhost"), получаем IP через gethostbyname
        // Берём первый IP-адрес из списка
        Daddr.sin_addr.s_addr = ((unsigned long**)hst->h_addr_list)[0][0];
    }
    else {
        // Не удалось определить адрес
        cout << "Unknown Host: " << WSAGetLastError() << "\n";
        closesocket(my_sock);
        WSACleanup();
        return -1;
    }
    
    // ========== ШАГ 4 - ОБМЕН СООБЩЕНИЙ С СЕРВЕРОМ ==========
    while (1) {  // Бесконечный цикл общения
        
        // ЧИТАЕМ СООБЩЕНИЕ С КЛАВИАТУРЫ
        cout << "S<=C:";  // Приглашение к вводу (S<=C - сообщение от клиента)
        string SS;        // Строка для хранения ввода пользователя
        getline(cin, SS); // Читаем всю строку
        
        if (SS == "quit") break;  // Если ввели "quit" - выходим из цикла
        
        // ОТПРАВЛЯЕМ СООБЩЕНИЕ НА СЕРВЕР
        // sendto() - отправка датаграммы
        // my_sock - наш сокет
        // (char*)&SS[0] - указатель на данные
        // SS.size() - размер данных
        // 0 - флаги
        // (sockaddr*)&Daddr - адрес получателя (сервера)
        // sizeof(Daddr) - размер адреса
        sendto(my_sock, (char*)&SS[0], SS.size(), 0,
               (sockaddr*)&Daddr, sizeof(Daddr));
        
        // ПРИНИМАЕМ ОТВЕТ ОТ СЕРВЕРА
        sockaddr_in SRaddr;         // Адрес отправителя (сервера)
        int SRaddr_size = sizeof(SRaddr);  // Размер адреса
        
        // recvfrom() - приём датаграммы
        int n = recvfrom(my_sock, buff, sizeof(buff), 0,
                         (sockaddr*)&SRaddr, &SRaddr_size);
        
        if (n == SOCKET_ERROR) {  // Проверка на ошибку
            cout << "RECVFROM() ERROR:" << WSAGetLastError() << "\n";
            closesocket(my_sock);
            WSACleanup();
            return -1;
        }
        
        buff[n] = '\0';  // Добавляем завершающий нуль для вывода как строки
        
        // ВЫВОДИМ ПОЛУЧЕННЫЙ ОТВЕТ НА ЭКРАН
        cout << "S=>C:" << buff << "\n";  // S=>C - сообщение от сервера
    }
    
    // ========== ШАГ ПОСЛЕДНИЙ - ВЫХОД ==========
    closesocket(my_sock);  // Закрываем сокет
    WSACleanup();          // Очищаем ресурсы WinSock
    return 0;              // Успешное завершение
}
