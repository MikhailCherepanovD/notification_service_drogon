#pragma once
#include <drogon/HttpController.h>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <iomanip>
using namespace std;
using namespace drogon;
using namespace drogon::orm;
#define LOG_DEBUG_C LOG_DEBUG<<" "<<getInfoRequest(req)
#define LOG_WARN_C LOG_WARN<<" "<<getInfoRequest(req)
#define LOG_ERROR_C LOG_ERROR<<" "<<getInfoRequest(req)
bool checkIsCorrectJson(const HttpRequestPtr &req,shared_ptr<HttpResponse> resp);
string getInfoRequest(const HttpRequestPtr &req);
void getResponseMissingRequiredFields(const HttpRequestPtr &req, shared_ptr<HttpResponse> resp);
tm parseDate(const string& dateStr, int& status);

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