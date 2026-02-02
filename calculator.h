#pragma once
#include <vector>
#include <thread>
#include <mutex>
/// Класс для вычисления интегралов с параллелизацией
class IntegrationCalculator {
public:
    /// Основная функция для вычисления интеграла a - Начало, b - Конец, h - Шаг интегрирования
    static double integrate(double a, double b, double h);

    /// Функция для вычисления подынтегральной функции f(x) = 1/ln(x)
    static double function(double x);
    static unsigned int getAvailableCores(); /// Определяет количество ядер процессора (минимум 4)
private:
    /// Вспомогательные методы
    static double integrateSegment(double a, double b, double h); /// Интегрирование одного отрезка методом прямоугольников
};