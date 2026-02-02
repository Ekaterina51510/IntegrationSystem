#include "calculator.h"
#include <iostream>
#include <cmath>
using namespace std;

/// Вычисление функции f(x) = 1/ln(x)
double IntegrationCalculator::function(double x) 
{
    if (fabs(x - 1.0) < 1e-10)
        return 0;
    return 1.0 / log(x);
}
/// Интегрирование одного отрезка
double IntegrationCalculator::integrateSegment(double a, double b, double h)
{
    double sum = 0;
    double x = a + h / 2.0;

    while (x < b)
    {
        sum += function(x);
        x += h;
    }
    return h * sum;
}
/// Определение количества ядер
unsigned int IntegrationCalculator::getAvailableCores()
{
    unsigned int cores = thread::hardware_concurrency();
    return (cores == 0) ? 4 : cores;
}

/// Параллельное интегрирование 
double IntegrationCalculator::integrate(double a, double b, double h)
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

    return integrateSegment(a, b, h);
}