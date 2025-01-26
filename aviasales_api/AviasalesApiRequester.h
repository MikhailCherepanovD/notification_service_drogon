#pragma once
#include <iostream>
#include <string>
#include "limits"
#include <fmt/core.h>
#include <drogon/HttpClient.h>
#include "controllers/controllers_utils.h"
using namespace std;
using namespace drogon;
// singleton class
class AviasalesApiRequester{
private:
    static const string TRAVELPAYOUTS_KEY;
    static const string WWW_REQUEST_HOST;  // используется: 1. Чтобы узнать AITA код, 2. Для вставки в начало возвращаемой Json ссылки;
    static const string API_REQUEST_HOST;  // используется: 1. Чтобы узнать информацию о перелете;
    Json::StreamWriterBuilder singletonJsonWriter;
    AviasalesApiRequester()    {
        LOG_DEBUG<<"Create singleton object AviasalesApiRequester";
    }
    ~AviasalesApiRequester(){
        LOG_DEBUG<<"Delete singleton object AviasalesApiRequester";
    }
    static AviasalesApiRequester* instance;
    class MemGuard{
    public:
        MemGuard(){
        }
        ~MemGuard(){
            delete instance;
            instance=nullptr;
        }
    };
public:
    AviasalesApiRequester(const AviasalesApiRequester&) = delete;
    AviasalesApiRequester& operator=(const AviasalesApiRequester&) = delete;
    static AviasalesApiRequester& getInstance()
    {
        static MemGuard g;
        if (!instance) {
            instance = new AviasalesApiRequester();
        }
        return *instance;
    }
    // status_executing = 0 - успешно, 1 - в базе данных нет городв с таким названием, 2 - ошибка выполнения запроса
    pair<string,string> getPairIATACode(const string&  origin, const string&  destination, int& status_executing);

    // status_executing = 0 - успешно, 1 - некорректные даты, 2 -  билетов не найдено
    Json::Value getJsonJourney(int& status_executing, const string&  originIata, const string&  destinationIata,
                               const tm& dateBegin, const tm& dateEnd,const string& directStr="true");

};

