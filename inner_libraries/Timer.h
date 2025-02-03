#pragma once
#include "iostream"
#include <ctime>
#include <chrono>
#include <iomanip>
using namespace std;
class Timer {
public:
    Timer() {
        // Сохраняем текущее время в момент создания объекта
        start_time = chrono::high_resolution_clock::now();
    }

    ~Timer() {
        // В момент уничтожения объекта вычисляем разницу
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);

        // Переводим разницу в секунды и миллисекунды
        double seconds = duration.count() / 1000000.0;

        // Выводим результат в формате сек.миллисек
        cout << "Time elapsed: "
             << fixed << setprecision(6) << seconds
             << " seconds" << endl;
    }
private:
    chrono::high_resolution_clock::time_point start_time;
};