// Клиент для численного интегрирования методом трапеций
// Клиент подключается к серверу, получает отрезок интегрирования и шаг,
// вычисляет интеграл функции 1 / ln(x) методом трапеций
// и возвращает результат на сервер.
//Метод трапеций :
//∫f(x)dx ≈ h/2 * [f(a) + 2Σf(x_i) + f(b)], где h - шаг интегрирования
#include <iostream>
#include <sstream>
#include <fstream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif
#include "calculator2.h"// Заголовочный файл с реализацией метода трапеций
using namespace std;

int main()
{
    setlocale(0, "");// Установка локали для корректного вывода русских символов
    setlocale(LC_NUMERIC, "C");// Установка "C" локали для чисел (точка как разделитель дробной части)
    ofstream logfile("client2.log");// Создание лог-файла для записи хода выполнения
    if (!logfile.is_open())
    {
        cout << "Ошибка создания лог-файла!" << endl;
        return 1;
    }
    cout << "ИНТЕГРАЦИОННЫЙ КЛИЕНТ(метод трапеции)\n\n";
    logfile << "ИНТЕГРАЦИОННЫЙ КЛИЕНТ(метод трапеции)" << endl;
    cout << "Инициализация сети...\n";
    logfile << "Инициализация сети..." << endl;
#ifdef _WIN32
    WSADATA wsaData;// Инициализация сетевой библиотеки (для Windows)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "ОШИБКА: Не получилось включить сеть!\n";
        logfile << "ОШИБКА: Не получилось включить сеть!" << endl;
        return 1;
    }
#endif
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);// Создание сокета для подключения к серверу
    if (clientSocket == INVALID_SOCKET)
    {
        cout << "ОШИБКА! Не удалось создать сокет." << "\n";
        logfile << "ОШИБКА! Не удалось создать сокет." << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    sockaddr_in serverAddr;// Настройка адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    cout << "Подключение к серверу...\n";
    logfile << "Подключение к серверу..." << endl;
    // Подключение к серверу
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "ОШИБКА: Не удалось подключиться к серверу!\n";
        logfile << "ОШИБКА: Не удалось подключиться к серверу!" << endl;
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "Подключение установлено. Ожидание задачи...\n";
    logfile << "Подключение установлено. Ожидание задачи..." << endl;
    int my_cores = thread::hardware_concurrency();// Определение количества ядер процессора для отправки серверу
    if (my_cores == 0) my_cores = 4;// Значение по умолчанию, если не удалось определить
    string cores_msg = to_string(my_cores) + "\n";
    send(clientSocket, cores_msg.c_str(), cores_msg.length(), 0);
    cout << "Сообщил серверу о " << my_cores << " ядрах\n";
    logfile << "Сообщил серверу о " << my_cores << " ядрах" << endl;
    char buffer[1024];// Получение задачи от сервера
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0)
    {
        cout << "ОШИБКА: Не получил задачу от сервера\n";
        logfile << "ОШИБКА: Не получил задачу от сервера" << endl;
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    buffer[bytesReceived] = '\0';
    string task_str = buffer;
    for (char& c : task_str) // Заменяем запятые на точки (если нужно)
    {
        if (c == ',') c = '.';
    }
    cout << "Получена задача от сервера: " << task_str;
    logfile << "Получена задача от сервера: " << task_str;
    double a, b, h; // Разбор полученной задачи
    stringstream ss(task_str);
    ss >> a >> b >> h;
    if (ss.fail())
    {
        cout << "ОШИБКА: Неверный формат задачи от сервера\n";
        logfile << "ОШИБКА: Неверный формат задачи от сервера" << endl;
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "Получена задача:\n";
    logfile << "Получена задача:" << endl;
    cout << "Интервал: [" << a << ", " << b << "]\n";
    logfile << "Интервал: [" << a << ", " << b << "]" << endl;
    cout << "Шаг: " << h << "\n";
    logfile << "Шаг: " << h << endl;
    cout << "\nВычисляю методом трапеций...\n";
    logfile << "Вычисляю методом трапеций..." << endl;
    double result = TrapezoidIntegrationCalculator::integrate(a, b, h);// Вычисление интеграла методом трапеций
    cout << "\nОтправляю результат серверу...\n";
    logfile << "Отправляю результат серверу..." << endl;
    string result_str = to_string(result) + "\n";// Преобразование результата в строку и добавление перевода строки
    send(clientSocket, result_str.c_str(), result_str.length(), 0);
    cout << "Результат отправлен: " << result << "\n";
    logfile << "Результат отправлен: " << result << endl;
    cout << "\nЗакрываю соединение...\n";
    logfile << "Закрываю соединение..." << endl;
    closesocket(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    cout << "Клиент (трапеций) завершил работу. Нажмите Enter...\n";
    logfile << "Клиент (трапеций) завершил работу. Нажмите Enter..." << endl;
    logfile.close();
    cin.get();
}