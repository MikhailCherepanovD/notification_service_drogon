#include "AviasalesApiRequester.h"


// status_executing = 0 - успешно, 1 - в базе данных нет городв с таким названием, 2 - ошибка выполнения запроса
pair<string,string> AviasalesApiRequester::getPairIATACode(const string&  origin, const string&  destination, int& status_executing){
    pair<string,string> ret;
    auto httpRequest = HttpRequest::newHttpRequest();
    httpRequest->setPath(fmt::format("/widgets_suggest_params?q=Из {} в {}",origin,destination));
    httpRequest->setMethod(Get);
    // в данном случае использование асинхронной схемы выполнения является избыточным, попробовал это здесь в рамках изучения.
    promise<void> prms;
    future<void> ftr = prms.get_future();
    HttpClient::newHttpClient(www_host)->sendRequest(
            httpRequest,  // Создаём HTTP-запрос
            [&prms,&status_executing,&ret,origin,destination](ReqResult result, const HttpResponsePtr &response) {
                if (result == ReqResult::Ok) {
                    try {
                        auto jo = response->getJsonObject();
                        string originIATA = (*jo)["origin"]["iata"].asString();
                        string destinationIATA = (*jo)["destination"]["iata"].asString();
                        status_executing = 0;
                        if(originIATA=="" || destinationIATA==""){
                            throw invalid_argument("Codes are empty");
                        }
                        ret.first=originIATA;
                        ret.second=destinationIATA;
                        LOG_INFO << fmt::format("Received IATA codes for cities:{} and {}",origin,destination);
                    }catch(...){
                        LOG_WARN << fmt::format("Wrong names for cities:{} or {}",origin,destination);
                        status_executing = 1;
                        ret.first="";
                        ret.second="";
                    }
                } else {
                    // Ошибка
                    status_executing = 2;
                    LOG_WARN << fmt::format("Request failed by cities:{},{}",origin,destination) << result;
                }
                prms.set_value();
            }
    );
    ftr.wait();
    return ret;
}


// status_executing = 0 - корректное исполнение, 1 - некорректные даты, 2 -  билетов не найдено
Json::Value AviasalesApiRequester::getJsonJourney(int& status_executing, const string&  originIata, const string&  destinationIata,
                           const tm& dateBegin, const tm& dateEnd,const string& directStr){
    Timer* timer = new Timer();
    vector<string> vecDate= getDatesBetween(dateBegin,dateEnd);
    int amountDate = vecDate.size();
    if(amountDate==0){
        status_executing=1;
        return {};
    }
    //string directStr = boolToStr(direct);

    auto funcSendRespPtr = [&]
            (const string&  originIata, const string&  destinationIata,const string& date,const string& direct ){
        auto httpRequest = HttpRequest::newHttpRequest();
        httpRequest->setPath(fmt::format("/aviasales/v3/prices_for_dates?origin={}&destination={}&departure_at={}&direct={}&limit=1&page=1&token={}",
                                         originIata, destinationIata, date, direct, api_key));
        auto pairResultResponse = HttpClient::newHttpClient(api_host)->sendRequest(httpRequest);
        //httpClientAviasalesPtr->sendRequest(httpRequest);
        auto result = pairResultResponse.first;
        auto response = pairResultResponse.second;
        //LOG_INFO<<"Полученные данные из запроса"<<response->getJsonObject()->toStyledString();
        shared_ptr<Json::Value> returnedJson= nullptr;
        if (result != ReqResult::Ok){
            LOG_WARN << fmt::format("Wrong code received from request");
            return returnedJson;
        }
        shared_ptr<Json::Value> receivedJson;
        receivedJson = response->getJsonObject();
        if((*receivedJson)["data"].empty()){
            LOG_WARN << fmt::format("Received empty array in json journey");
            return returnedJson;
        }

        string returnedLink = www_host + "/" + (*receivedJson)["data"][0]["link"].asString();
        (*receivedJson)["data"][0]["link"]=returnedLink;
        returnedJson = make_shared<Json::Value>((*receivedJson)["data"][0]);//(*receivedJson)["data"][0];

        return returnedJson;
    };

    vector<future<shared_ptr<Json::Value>>> vecFuture(amountDate);
    for(int i=0;i<amountDate;i++){
        vecFuture[i] = std::async(launch::async,funcSendRespPtr,originIata,destinationIata,vecDate[i],directStr);
    }
    int minPrice = numeric_limits<int>::max();
    Json::Value returnedValue={};
    for(int i=0;i<amountDate;i++){
        auto receivedJsonValue=vecFuture[i].get();
        if(receivedJsonValue){
            if((*receivedJsonValue)["price"].asInt()<minPrice){
                minPrice = (*receivedJsonValue)["price"].asInt();
                returnedValue = (*receivedJsonValue);
            }
        }
    }
    delete timer;
    if(returnedValue.empty()){
        status_executing=2;
    }else{
        status_executing=0;
    }
    return returnedValue;
}
