//Сервер принимает параметры интегрирования от пользователя, распределяет
//задачи между подключенными клиентами пропорционально их вычислительным
//мощностям(количеству ядер CPU) и суммирует полученные результаты.
//Протокол взаимодействия :
//1. Клиент подключается и отправляет количество своих ядер
//2. Сервер распределяет отрезок интегрирования между клиентами
//3. Сервер отправляет каждому клиенту его отрезок и шаг
//4. Клиенты вычисляют интеграл на своих отрезках и возвращают результат
//5. Сервер суммирует все результаты и выводит итог
#include <iostream>
#include <string> 
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
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
using namespace std;
int main()
{
    setlocale(0, ""); // Установка локали для корректного вывода русских символов
    setlocale(LC_NUMERIC, "C");// Установка "C" локали для чисел (точка как разделитель дробной части)
    ofstream logfile("server.log");// Открытие лог-файла для записи хода выполнения
    if (!logfile.is_open())
    {
        cout << "Ошибка создания лог-файла сервера!" << endl;
        return 1;
    }
    cout << "ИНТЕГРАЦИОННЫЙ СЕРВЕР \n\n";
    logfile << "ИНТЕГРАЦИОННЫЙ СЕРВЕР" << endl;
    double a, b, h;// Параметры интегрирования
    cout << "Введите общие параметры задачи:\n";
    logfile << "Ввод параметров задачи:" << endl;
    cout << "Начало (a): "; cin >> a;
    cout << "Конец (b): ";  cin >> b;
    cout << "Шаг (h): ";    cin >> h;
    logfile << "a = " << a << ", b = " << b << ", h = " << h << endl;
    if (a >= b || h <= 0) // Проверка корректности введенных параметров
    {
        cout << "Ошибка: некорректные параметры!\n";
        logfile << "Ошибка: некорректные параметры!" << endl;
        return 1;
    }
    int num_clients;// Количество клиентов для подключения
    cout << "Сколько клиентов будет подключаться? ";
    cin >> num_clients;
    logfile << "Количество клиентов: " << num_clients << endl;
    if (num_clients <= 0) 
    {
        cout << "Ошибка: нужно хотя бы 1 клиент!\n";
        logfile << "Ошибка: нужно хотя бы 1 клиент!" << endl;
        return 1;
    }
    
    cout << "Запуск сервера...\n";
    logfile << "Запуск сервера..." << endl;
#ifdef _WIN32
    WSADATA wsaData;// Инициализация сетевой библиотеки (для Windows)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "Ошибка инициализации сети!\n";
        logfile << "Ошибка инициализации сети!" << endl;
        return 1;
    }
#endif
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);// Создание сокета сервера
    if (serverSocket == INVALID_SOCKET) 
    {
        cout << "Ошибка создания сокета!" << "\n";
        logfile << "Ошибка создания сокета!" << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    sockaddr_in serverAddr;// Настройка адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // Привязка сокета к адресу
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        cout << "Ошибка привязки сокета!" << "\n";
        logfile << "Ошибка привязки сокета!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "Сервер запущен на порту 12345\n";
    logfile << "Сервер запущен на порту 12345" << endl;
    if (listen(serverSocket, 5) == SOCKET_ERROR) // Начало прослушивания входящих соединений
    {
        cout << "Ошибка: не удалось начать прослушивание!" << "\n";
        logfile << "Ошибка: не удалось начать прослушивание!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    vector<SOCKET> client_sockets;// Векторы для хранения информации о подключенных клиентах
    vector<int> client_cores;
    int total_cores = 0;
    for (int i = 0; i < num_clients; i++) // Ожидание подключения всех клиентов
    {
        cout << "Ожидание подключения клиента " << (i + 1) << "...\n";
        logfile << "Ожидание подключения клиента " << (i + 1) << "..." << endl;
        sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
        if (clientSocket == INVALID_SOCKET) 
        {
            cout << "Ошибка подключения клиента!\n";
            logfile << "Ошибка подключения клиента!" << endl;
            closesocket(serverSocket);
#ifdef _WIN32
            WSACleanup();
#endif
            return 1;
        }
        // Получение информации о количестве ядер клиента
        char cores_buffer[32];
        int cores_received = recv(clientSocket, cores_buffer, sizeof(cores_buffer) - 1, 0);
        int cores = 4;  /// значение по умолчанию

        if (cores_received > 0) {
            cores_buffer[cores_received] = '\0';
            cores = atoi(cores_buffer);
            if (cores <= 0) cores = 4;
        }
        // Сохранение информации о клиенте
        client_sockets.push_back(clientSocket);
        client_cores.push_back(cores);
        total_cores += cores;

        cout << "Клиент " << (i + 1) << " подключен (" << cores << " ядер)\n";
        logfile << "Клиент " << (i + 1) << " подключен (" << cores << " ядер)" << endl;
    }
    cout << "\nРаспределение задач (пропорционально ядрам):\n";
    logfile << "Распределение задач (пропорционально ядрам):" << endl;
    // Распределение отрезков интегрирования между клиентами
    vector<pair<double, double>> client_tasks;
    double current_position = a;
    // Вычисление доли работы для данного клиента
    for (int i = 0; i < client_sockets.size(); i++)
    {
        // Доля работы = ядра_клиента / общее_количество_ядер
        double share = static_cast<double>(client_cores[i]) / total_cores;
        double segment_length = (b - a) * share;

        double start = current_position;
        double end = (i == client_sockets.size() - 1) ? b : start + segment_length;

        client_tasks.push_back({ start, end });

        cout << "Клиент " << (i + 1) << ": [" << start << ", " << end
            << "] (ядер: " << client_cores[i] << ", доля: "
            << fixed << setprecision(1) << (share * 100) << "%)\n";

        logfile << "Клиент " << (i + 1) << ": [" << start << ", " << end
            << "] (ядер: " << client_cores[i] << ", доля: "
            << fixed << setprecision(1) << (share * 100) << "%)" << endl;

        current_position = end;
    }
    cout << "\nОтправка задач...\n";// Отправка задач всем клиентам
    logfile << "Отправка задач..." << endl;
    for (int i = 0; i < client_sockets.size(); i++) 
    {
        string task_msg = to_string(client_tasks[i].first) + " " +
            to_string(client_tasks[i].second) + " " +
            to_string(h) + "\n";
        send(client_sockets[i], task_msg.c_str(), task_msg.length(), 0);
    }
    cout << "Все задачи отправлены! Ожидание результатов...\n\n";
    logfile << "Все задачи отправлены! Ожидание результатов..." << endl;
    // Получение и суммирование результатов от клиентов
    double total_result = 0;
    for (int i = 0; i < client_sockets.size(); i++)
    {
        char buffer[1024];
        int bytesReceived = recv(client_sockets[i], buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            string result_str = buffer;
            double client_result = atof(result_str.c_str());
            cout.precision(10);
            cout << "  Клиент " << (i + 1) << " вернул: " << client_result << "\n";
            logfile << "Клиент " << (i + 1) << " вернул: " << client_result << endl;
            total_result += client_result;
        }
        else
        {
            cout << "  Клиент " << (i + 1) << " не вернул результат\n";
            logfile << "Клиент " << (i + 1) << " не вернул результат" << endl;
        }
        closesocket(client_sockets[i]);
    }
    cout.precision(10);// Вывод итоговых результатов
    cout << "\n================================\n";
    cout << "ИНТЕГРИРОВАНИЕ ЗАВЕРШЕНО\n";
    cout << "Функция: 1 / ln(x)\n";
    cout << "Интервал: [" << a << ", " << b << "]\n";
    cout << "Шаг: " << h << "\n";
    cout << "Клиентов: " << num_clients << "\n";
    cout << "РЕЗУЛЬТАТ: " << total_result << "\n";
    cout << "================================\n";
    // Запись итогов в лог-файл
    logfile << "\n========================================" << endl;
    logfile << "ИНТЕГРИРОВАНИЕ ЗАВЕРШЕНО" << endl;
    logfile << "Функция: 1 / ln(x)" << endl;
    logfile << "Интервал: [" << a << ", " << b << "]" << endl;
    logfile << "Шаг: " << h << endl;
    logfile << "Клиентов: " << num_clients << endl;
    logfile << "РЕЗУЛЬТАТ: " << total_result << endl;
    logfile << "========================================" << endl;

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    cout << "Сервер завершил работу. Нажмите Enter...\n";
    logfile << "Сервер завершил работу." << endl;

    logfile.close();
    cin.get();
}
