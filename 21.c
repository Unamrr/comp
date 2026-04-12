#include <stdafx.h>
#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string>
#include <windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

#define SRV_HOST "localhost"
#define SRV_PORT 1234
#define CLNT_PORT 1235
#define BUF_SIZE 64

int main() {
    char buff[1024];
    
    // Запускаем WinSock
    if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }
    
    SOCKET s;
    int from_len;
    char buf[BUF_SIZE] = { 0 };
    hostent* hp;
    sockaddr_in clnt_sin, srv_sin;
    
    // Создаём сокет
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    // Настройка клиентского адреса
    clnt_sin.sin_family = AF_INET;
    clnt_sin.sin_addr.s_addr = 0;
    clnt_sin.sin_port = htons(CLNT_PORT);
    bind(s, (sockaddr*)&clnt_sin, sizeof(clnt_sin));
    
    // Подключаемся к серверу
    hp = gethostbyname(SRV_HOST);
    srv_sin.sin_port = htons(SRV_PORT);
    srv_sin.sin_family = AF_INET;
    ((unsigned long*)&srv_sin.sin_addr)[0] = ((unsigned long**)hp->h_addr_list)[0][0];
    connect(s, (sockaddr*)&srv_sin, sizeof(srv_sin));
    
    string choice;
    
    cout << "=== ИГРА КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===" << endl;
    cout << "Подключение к серверу..." << endl;
    
    do {
        // Получаем сообщение от сервера
        from_len = recv(s, (char*)&buf, BUF_SIZE, 0);
        buf[from_len] = 0;
        cout << buf;  // Выводим приглашение от сервера
        
        // Читаем выбор игрока
        getline(cin, choice);
        
        // Отправляем выбор серверу
        int msg_size = choice.size();
        send(s, (char*)&choice[0], msg_size, 0);
        
    } while (choice != "Bye");
    
    cout << "Игра завершена. До свидания!" << endl;
    cin.get();
    closesocket(s);
    
    return 0;
}

#include <stdafx.h>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <cstdlib>
#include <ctime>
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define SRV_PORT 1234
#define BUF_SIZE 64

int main() {
    // Инициализируем генератор случайных чисел для выбора сервера
    srand(time(0));
    
    char buff[1024];
    
    // Запускаем WinSock
    if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }
    
    SOCKET s, s_new;
    int from_len;
    char buf[BUF_SIZE] = { 0 };
    sockaddr_in sin, from_sin;
    
    // Создаём сокет
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    // Настройка адреса сервера
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;  // Слушаем все интерфейсы
    sin.sin_port = htons(SRV_PORT);
    bind(s, (sockaddr*)&sin, sizeof(sin));
    
    // Начинаем слушать подключения
    listen(s, 3);
    
    cout << "=== СЕРВЕР КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===" << endl;
    cout << "Ожидание подключения игрока..." << endl;
    
    while (1) {
        from_len = sizeof(from_sin);
        s_new = accept(s, (sockaddr*)&from_sin, &from_len);
        cout << "Игрок подключился!" << endl;
        
        string msg, player_choice;
        
        while (1) {
            // Отправляем приглашение игроку
            msg = "\n=== ВАШ ХОД ===\n1 - Камень\n2 - Ножницы\n3 - Бумага\nBye - Выйти из игры\nВаш выбор: ";
            send(s_new, (char*)&msg[0], msg.size(), 0);
            
            // Получаем выбор игрока
            from_len = recv(s_new, (char*)buf, BUF_SIZE, 0);
            buf[from_len] = 0;
            player_choice = (string)buf;
            
            // Удаляем лишние символы перевода строки
            if (!player_choice.empty() && player_choice.back() == '\n') {
                player_choice.pop_back();
            }
            
            cout << "Игрок выбрал: " << player_choice << endl;
            
            // Проверка на выход
            if (player_choice == "Bye") {
                break;
            }
            
            // Сервер делает случайный выбор (1, 2 или 3)
            int server_choice = rand() % 3 + 1;
            string server_str;
            
            // Определяем, что выбрал сервер
            if (server_choice == 1) server_str = "КАМЕНЬ";
            else if (server_choice == 2) server_str = "НОЖНИЦЫ";
            else server_str = "БУМАГА";
            
            // Формируем результат игры
            string result;
            
            if (player_choice == "1") {  // Игрок выбрал камень
                if (server_choice == 1) {
                    result = "НИЧЬЯ! Сервер тоже выбрал камень.";
                } else if (server_choice == 2) {
                    result = "ВЫ ВЫИГРАЛИ! Камень разбивает ножницы!";
                } else {
                    result = "ВЫ ПРОИГРАЛИ! Бумага накрывает камень!";
                }
            }
            else if (player_choice == "2") {  // Игрок выбрал ножницы
                if (server_choice == 2) {
                    result = "НИЧЬЯ! Сервер тоже выбрал ножницы.";
                } else if (server_choice == 3) {
                    result = "ВЫ ВЫИГРАЛИ! Ножницы режут бумагу!";
                } else {
                    result = "ВЫ ПРОИГРАЛИ! Камень ломает ножницы!";
                }
            }
            else if (player_choice == "3") {  // Игрок выбрал бумагу
                if (server_choice == 3) {
                    result = "НИЧЬЯ! Сервер тоже выбрал бумагу.";
                } else if (server_choice == 1) {
                    result = "ВЫ ВЫИГРАЛИ! Бумага накрывает камень!";
                } else {
                    result = "ВЫ ПРОИГРАЛИ! Ножницы режут бумагу!";
                }
            }
            else {  // Неправильный ввод
                result = "ОШИБКА! Введите 1, 2, 3 или Bye";
            }
            
            // Отправляем результат игроку
            msg = "\nСервер выбрал: " + server_str + "\n" + result + "\n";
            send(s_new, (char*)&msg[0], msg.size(), 0);
        }
        
        cout << "Игрок отключился" << endl;
        closesocket(s_new);
    }
    
    closesocket(s);
    return 0;
}



#include <stdafx.h>  // Предкомпилированный заголовок Visual Studio

#include <iostream>  // Для cin, cout - ввод/вывод в консоль

#define _WINSOCK_DEPRECATED_NO_WARNINGS  // Отключаем предупреждения об устаревших функциях winsock

// подавление предупреждений библиотеки winsock2
#include <winsock2.h>  // Основная библиотека для работы с сокетами в Windows

#include <string>  // Для работы с типом string (строки)

#include <windows.h>  // Дополнительные функции Windows

#pragma comment (lib, "Ws2_32.lib")  // Указываем компилятору подключить библиотеку сокетов

#pragma warning(disable: 4996)  // Отключаем конкретное предупреждение 4996 (об устаревших функциях)

using namespace std;  // Чтобы не писать std:: перед cout, cin, string

// ---------- КОНСТАНТЫ ----------
#define SRV_HOST "localhost"  // Адрес сервера (localhost = этот же компьютер)
#define SRV_PORT 1234         // Порт сервера (должен совпадать с сервером)
#define CLNT_PORT 1235        // Порт клиента (можно любой свободный)
#define BUF_SIZE 64           // Размер буфера для приёма данных

int main() {  // Главная функция - точка входа в программу
    
    char buff[1024];  // Временный буфер для передачи параметров в WSAStartup
    
    // ---------- ЗАПУСК WINSAUCK API ----------
    // WSAStartup - инициализирует работу с сокетами в Windows
    // 0x0202 - версия WinSock 2.2
    // (WSADATA*)&buff[0] - структура, куда сохранится информация о версии
    if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
        // Если WSAStartup вернул НЕ 0 (ошибка) - выводим сообщение и код ошибки
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;  // Завершаем программу с кодом ошибки
    }
    
    // ---------- ОБЪЯВЛЕНИЕ ПЕРЕМЕННЫХ ----------
    SOCKET s;           // Дескриптор (идентификатор) нашего сокета
    int from_len;       // Переменная для хранения количества принятых байт
    char buf[BUF_SIZE] = { 0 };  // Буфер для приёма данных (заполнен нулями)
    hostent* hp;        // Указатель на структуру с информацией о хосте (сервере)
    
    sockaddr_in clnt_sin, srv_sin;  // Структуры для адресов клиента и сервера
    
    // ---------- СОЗДАНИЕ СОКЕТА ----------
    // socket() - создаёт новый сокет
    // AF_INET - семейство протоколов IPv4
    // SOCK_STREAM - тип сокета для TCP (надёжная передача данных)
    // 0 - протокол по умолчанию (TCP)
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    // ---------- НАСТРОЙКА АДРЕСА КЛИЕНТА ----------
    clnt_sin.sin_family = AF_INET;        // Семейство адресов IPv4
    clnt_sin.sin_addr.s_addr = 0;         // 0 = INADDR_ANY (любой сетевой интерфейс)
    clnt_sin.sin_port = htons(CLNT_PORT); // Порт клиента (преобразованный в сетевой порядок байт)
    
    // ---------- ПРИВЯЗКА СОКЕТА К ПОРТУ ----------
    // bind() - привязывает сокет к конкретному порту и адресу
    // (sockaddr*)&clnt_sin - указатель на структуру с адресом
    // sizeof(clnt_sin) - размер структуры
    bind(s, (sockaddr*)&clnt_sin, sizeof(clnt_sin));
    
    // ---------- ПОЛУЧЕНИЕ IP-АДРЕСА СЕРВЕРА ----------
    // gethostbyname() - преобразует имя хоста (например "localhost") в IP-адрес
    hp = gethostbyname(SRV_HOST);
    
    // ---------- НАСТРОЙКА АДРЕСА СЕРВЕРА ----------
    srv_sin.sin_port = htons(SRV_PORT);   // Порт сервера (в сетевом порядке)
    srv_sin.sin_family = AF_INET;         // Семейство адресов IPv4
    
    // Сложная конструкция для копирования IP-адреса из hp в srv_sin
    // Берём первый IP-адрес из списка адресов хоста
    ((unsigned long*)&srv_sin.sin_addr)[0] = 
        ((unsigned long**)hp->h_addr_list)[0][0];
    
    // ---------- ПОДКЛЮЧЕНИЕ К СЕРВЕРУ ----------
    // connect() - устанавливает соединение с сервером
    connect(s, (sockaddr*)&srv_sin, sizeof(srv_sin));
    
    // ---------- ОСНОВНОЙ ЦИКЛ ИГРЫ ----------
    string choice;  // Строка для хранения выбора игрока
    
    // Приветствие игрока
    cout << "=== ИГРА КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===" << endl;
    cout << "Подключение к серверу..." << endl;
    
    do {  // Начинаем цикл (будет выполняться пока игрок не введёт "Bye")
        
        // ---------- ПРИЁМ ДАННЫХ ОТ СЕРВЕРА ----------
        // recv() - получает данные от сервера
        // s - наш сокет
        // (char*)&buf - буфер для приёма
        // BUF_SIZE - максимальный размер буфера
        // 0 - флаги (не используем)
        from_len = recv(s, (char*)&buf, BUF_SIZE, 0);
        
        buf[from_len] = 0;  // Добавляем нуль-терминатор в конец строки
        cout << buf;        // Выводим сообщение от сервера (приглашение сделать ход)
        
        // ---------- ВВОД ВЫБОРА ИГРОКА ----------
        getline(cin, choice);  // Читаем всю строку, что ввёл игрок (1, 2, 3 или Bye)
        
        // ---------- ОТПРАВКА ДАННЫХ СЕРВЕРУ ----------
        int msg_size = choice.size();  // Узнаём размер сообщения
        // send() - отправляет данные серверу
        // (char*)&choice[0] - указатель на первый символ строки
        // msg_size - сколько байт отправляем
        // 0 - флаги
        send(s, (char*)&choice[0], msg_size, 0);
        
    } while (choice != "Bye");  // Повторяем, пока не ввели "Bye" (выход)
    
    // ---------- ЗАВЕРШЕНИЕ ----------
    cout << "Игра завершена. До свидания!" << endl;
    cin.get();           // Ждём нажатия Enter (чтобы консоль не закрылась сразу)
    closesocket(s);      // Закрываем сокет
    
    return 0;  // Успешное завершение программы
}

