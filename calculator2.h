#pragma once
#include <vector>
#include <thread>
#include <mutex>
class TrapezoidIntegrationCalculator {
public:
    /// Вычисление интеграла методом трапеций
    static double integrate(double a, double b, double h);

    /// Функция для вычисления подынтегральной функции f(x) = 1/ln(x)
    static double function(double x);

    /// Получение количества ядер (если нужно для логирования)
    static unsigned int getAvailableCores();
};