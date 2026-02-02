#include "calculator2.h"
#include <iostream>
#include <cmath>
using namespace std;
/// Метод трапеций
double TrapezoidIntegrationCalculator::integrate(double a, double b, double h)
{
    if (a >= b)
    {
        cerr << "Ошибка: начало интервала должно быть меньше конца!\n";
        return 0;
    }
    if (h <= 0)
    {
        cerr << "Ошибка: шаг должен быть положительным!\n";
        return 0;
    }

    double sum = (function(a) + function(b)) / 2.0;
    double x = a + h;

    while (x < b)
    {
        sum += function(x);
        x += h;
    }

    return h * sum;
}
/// Вычисление функции f(x) = 1/ln(x)
double TrapezoidIntegrationCalculator::function(double x)
{
    if (fabs(x - 1.0) < 1e-10)
        return 0;
    return 1.0 / log(x);
}
/// Получение ядер
unsigned int TrapezoidIntegrationCalculator::getAvailableCores()
{
    unsigned int cores = thread::hardware_concurrency();
    return (cores == 0) ? 4 : cores;
}