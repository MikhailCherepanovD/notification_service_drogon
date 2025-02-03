#pragma once
#include <iostream>
#include <string>
#include "limits"
#include <fmt/core.h>
#include <drogon/HttpClient.h>
#include "inner_libraries/time_and_json_utils.h"
#include "inner_libraries/Timer.h"
#include "inner_libraries/Singleton.h"
#include "inner_libraries/ConfigReader.h"
using namespace std;
using namespace drogon;
class AviasalesApiRequester:public Singleton<AviasalesApiRequester>{
private:
    string api_key;
    string www_host;  // используется: 1. Чтобы узнать AITA код, 2. Для вставки в начало возвращаемой Json ссылки;
    string api_host;  // используется: 1. Чтобы узнать информацию о перелете;
    ConfigReader &configReader;
    Json::StreamWriterBuilder singletonJsonWriter;

public:
    AviasalesApiRequester():
    configReader(ConfigReader::getInstance()){
        api_key = (*configReader.getJsonValue())["travelpayouts_api"]["key"].asString();
        www_host = (*configReader.getJsonValue())["travelpayouts_api"]["www_host"].asString();
        api_host = (*configReader.getJsonValue())["travelpayouts_api"]["api_host"].asString();
    }

    // status_executing = 0 - успешно, 1 - в базе данных нет городв с таким названием, 2 - ошибка выполнения запроса
    pair<string,string> getPairIATACode(const string&  origin, const string&  destination, int& status_executing);

    // status_executing = 0 - успешно, 1 - некорректные даты, 2 -  билетов не найдено
    Json::Value getJsonJourney(int& status_executing, const string&  originIata, const string&  destinationIata,
                               const tm& dateBegin, const tm& dateEnd,const string& directStr="true");

};

