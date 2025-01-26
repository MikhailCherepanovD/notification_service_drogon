#include "Routes.h"

bool Routes::checkIsCorrectJsonFieldForRoute( unordered_map<string,string>& jsonDict,shared_ptr<HttpRequest>req, shared_ptr<HttpResponse>resp,
                                              tm& dateBegin,tm& dateEnd){
    Json::Value retJsonValue;
    for(auto& it:jsonDict){
        if(it.second==""){
            resp->setStatusCode(k400BadRequest);
            LOG_DEBUG_C<<fmt::format("Received bad request. Missing required fields" );
            return false;
        }
    }
    if(allowedTypesOfJorney.find(jsonDict["type_of_journey"])==allowedTypesOfJorney.end()){
        resp->setStatusCode(k400BadRequest);
        LOG_DEBUG_C<<fmt::format("not allowed type of journey");
        retJsonValue["message"]="not allowed type of journey";
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        return false;
    }
    if(jsonDict["direct"]!="true" &&jsonDict["direct"]!="TRUE" && jsonDict["direct"]!="false" && jsonDict["direct"]!="FALSE"){
        LOG_DEBUG_C<<fmt::format("not allowed field direct");
        retJsonValue["message"]="not allowed field direct";
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        return false;
    }
    if(allowedTypesOfFrequencyOfMonitoring.find(jsonDict["type_frequency_of_monitoring"])==allowedTypesOfFrequencyOfMonitoring.end()){
        resp->setStatusCode(k400BadRequest);
        LOG_DEBUG_C<<fmt::format("not allowed type frequency of monitoring");
        retJsonValue["message"]="not allowed type frequency of monitoring";
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        return false;
    }
    int statusDate1;
    int statusDate2;
    dateBegin = parseDate(jsonDict["begin_date_monitoring"],statusDate1);
    dateEnd = parseDate(jsonDict["end_date_monitoring"],statusDate2);
    if(statusDate1==0 || statusDate2==0){
        resp->setStatusCode(k400BadRequest);
        LOG_DEBUG_C<<fmt::format("could not parse date");
        retJsonValue["message"]="could not parse date";
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        return false;
    }

    return true;
}

std::string getCurrentDateAndTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Преобразуем в структуру tm
    std::tm local_tm = *std::localtime(&now_time);

    // Форматируем время в строку
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

unordered_map<string,string> getJsonDictByJourney(shared_ptr<Json::Value> jo){
    unordered_map<string,string> jsonDict;
    jsonDict["origin"] = (*jo)["origin"].asString();
    jsonDict["destination"] = (*jo)["destination"].asString();
    jsonDict["type_of_journey"] = (*jo)["type_of_journey"].asString();
    jsonDict["frequency_of_monitoring"] = (*jo)["frequency_of_monitoring"].asString();
    jsonDict["begin_date_monitoring"] = (*jo)["begin_date_monitoring"].asString();
    jsonDict["end_date_monitoring"] = (*jo)["end_date_monitoring"].asString();
    jsonDict["direct"] = (*jo)["direct"].asString();
    jsonDict["type_frequency_of_monitoring"]= (*jo)["type_frequency_of_monitoring"].asString() == ""?
                                              "hours":(*jo)["type_frequency_of_monitoring"].asString(); // если поле не передано - по умолчанию будет задано hours
    return jsonDict;
}

Json::Value Routes::getJsonItemByRow(const HttpRequestPtr& req,const Row& row){
    Json::Value item;
    item["route_monitoring_id"] = row["route_monitoring_id"].as<int>();
    item["frequency_monitoring"] = row["frequency_monitoring"].as<int>();
    item["start_time_monitoring"] = row["start_time_monitoring"].as<std::string>();
    item["finish_time_monitoring"] = row["finish_time_monitoring"].as<std::string>();
    item["start_city"] = row["start_city"].as<std::string>();
    item["start_iata"] = row["start_iata"].as<std::string>();
    item["finish_city"] = row["finish_city"].as<std::string>();
    item["finish_iata"] = row["finish_iata"].as<std::string>();
    LOG_DEBUG_C<<"City "<< item["start_city"].as<std::string>();
    return item;
}
bool Routes::userIsExists(shared_ptr<HttpResponse>resp,shared_ptr<HttpRequest>req, string userId){
    auto f1 = dbClient->execSqlAsyncFuture(R"(SELECT COUNT(*) FROM users WHERE users_id = $1)", userId);
    try{
        auto result = f1.get();
        int amountUsers = result.at(0)["count"].as<int>();
        if(amountUsers==0){
            resp->setStatusCode(k404NotFound);
            LOG_DEBUG_C<<"User not found";
            return false;
        }
    }catch (...){
        LOG_ERROR_C<<"Unknown error in database";
        resp->setStatusCode(k500InternalServerError);
        return false;
    }
    return true;
}

bool Routes::routeIsExists(shared_ptr<HttpResponse>resp,shared_ptr<HttpRequest>req, string userId,string routeId){
    auto f1 = dbClient->execSqlAsyncFuture(R"(SELECT COUNT(*) FROM route_monitoring WHERE users_id = $1 AND route_monitoring_id=$2;)",userId,routeId);
    try{
        auto result = f1.get();
        int amountUsers = result.at(0)["count"].as<int>();
        if(amountUsers==0){
            resp->setStatusCode(k404NotFound);
            LOG_DEBUG_C<<"Route not found";
            return false;
        }
    }catch (...){
        LOG_ERROR_C<<"Unknown error in database";
        resp->setStatusCode(k500InternalServerError);
        return false;
    }
    return true;
}

shared_ptr<HttpResponse> Routes::postOrPutRoute(const HttpRequestPtr &req, string& userId, string*  routeIdPtr){
    Json::Value retJsonValue;
    auto resp=HttpResponse::newHttpResponse();
    if(!checkIsCorrectJson(req,resp)){
        return resp;
    }
    auto jo =req->getJsonObject();
    unordered_map<string,string> jsonDict = getJsonDictByJourney(jo);
    std::tm dateBegin={};
    std::tm dateEnd={};
    if(!checkIsCorrectJsonFieldForRoute(jsonDict,req,resp,dateBegin,dateEnd)){
        return resp;
    }

    int statusRequest;
    pair<string,string> pairIataCodes=aviasalesApiRequester.getPairIATACode(jsonDict["origin"], jsonDict["destination"], statusRequest);
    if(statusRequest==1){
        resp->setStatusCode(k400BadRequest);
        LOG_WARN_C<<"Error iata code request from database";
        retJsonValue["message"]="Wrong names of cities";
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        return resp;
    }
    else if(statusRequest==2){
        LOG_WARN_C<<"Unknown error iata code request";
        resp->setStatusCode(k500InternalServerError);
        return resp;
    }

    auto journeyInfo = aviasalesApiRequester.getJsonJourney(statusRequest, pairIataCodes.first,
                                                            pairIataCodes.second, dateBegin, dateEnd,jsonDict["direct"]);
    if(statusRequest==1){
        LOG_WARN_C<<"Dates is incorrect";
        resp->setStatusCode(k422UnprocessableEntity);
        retJsonValue["message"]="Dates is incorrect";
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        return resp;
    }
    if(statusRequest==2){
        LOG_WARN_C<<"There are not tickets for these dates";
        resp->setStatusCode(k204NoContent);
        return resp;
    }
    future<Result> f;
    if(routeIdPtr) {// Post
        f = dbClient->execSqlAsyncFuture(R"(
        SELECT * FROM insert_data_journey($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13))", userId,
                                         jsonDict["frequency_of_monitoring"], jsonDict["begin_date_monitoring"],
                                         jsonDict["end_date_monitoring"], jsonDict["direct"],
                                         jsonDict["type_of_journey"],
                                         jsonDict["origin"], pairIataCodes.first,
                                         jsonDict["destination"], pairIataCodes.second,
                                         getCurrentDateAndTime(), journeyInfo.toStyledString(),
                                         *routeIdPtr);
    }
    else{ //Put
        f = dbClient->execSqlAsyncFuture(R"(
        SELECT * FROM insert_data_journey($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12))", userId,
                                         jsonDict["frequency_of_monitoring"], jsonDict["begin_date_monitoring"],
                                         jsonDict["end_date_monitoring"], jsonDict["direct"],
                                         jsonDict["type_of_journey"],
                                         jsonDict["origin"], pairIataCodes.first,
                                         jsonDict["destination"], pairIataCodes.second,
                                         getCurrentDateAndTime(), journeyInfo.toStyledString());
    }

    //auto f = dbClient->execSqlAsyncFuture(R"(SELECT * FROM users;)");
    try{
        auto result=f.get();
        auto r1= result.at(0);
        const auto& columns = result.columns();

        auto r2 = result.at(0)["status"];
        int statusRequest = result.at(0)["status"].as<int>();
        int routeId = result.at(0)["returning_route_monitoring_id"].as<int>();
        if(statusRequest==3){
            LOG_ERROR_C<<"Unknown error in database";
            resp->setStatusCode(k500InternalServerError);
            return resp;
        }else if(statusRequest==0){
            LOG_WARN_C<<"User not found";
            resp->setStatusCode(k404NotFound);
            retJsonValue["message"]="User not found";
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
            return resp;
        }else if(statusRequest==1){
            LOG_DEBUG_C<<"Data journey suceesfully added";
            resp->setStatusCode(k201Created);
            retJsonValue=journeyInfo;
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
            resp->addHeader("Location", fmt::format(routesUrl,userId,routeId));
            return resp;
        }else if(statusRequest==2){
            LOG_DEBUG_C<<"Data journey suceesfully updated";
            resp->setStatusCode(k200OK);
            retJsonValue=journeyInfo;
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
            resp->addHeader("Location", fmt::format(routesUrl,userId,routeId));
            return resp;
        }
    }catch(const DrogonDbException &e){
        LOG_ERROR_C<<"Unknown error in database";
        resp->setStatusCode(k500InternalServerError);
        return resp;
    }
}



void Routes::postRoute(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback, string &&userId){
    auto resp = postOrPutRoute(req,userId);
    callback(resp);
}

void Routes::getRoutes(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback, string &&userId){
    Json::Value retJsonValue;
    auto resp=HttpResponse::newHttpResponse();
    if(!userIsExists(resp,req,userId)){
        callback(resp);
        return;
    }
    auto f2 = dbClient->execSqlAsyncFuture(
            R"(SELECT * FROM get_all_routes_by_user($1);)", userId);
    try{
        auto result = f2.get();
        for (const auto& row : result) {
            Json::Value item = getJsonItemByRow(req,row);
            retJsonValue.append(item); // Добавляем объект в массив
        }
    }catch (...){
        LOG_ERROR_C<<"Unknown error in database";
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }
    LOG_DEBUG_C<<"Get successful";
    resp->setStatusCode(k200OK);
    resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
    callback(resp);
}

void Routes::putRoute(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId){
    auto resp = postOrPutRoute(req,userId,&routeId);
    callback(resp);
}
void Routes::getRoute(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId){

    Json::Value retJsonValue;
    auto resp=HttpResponse::newHttpResponse();
    if(!userIsExists(resp,req,userId)){
        callback(resp);
        return;
    }
    if(!routeIsExists(resp,req,userId,routeId)){
        callback(resp);
        return;
    }
    auto f2 = dbClient->execSqlAsyncFuture(
            R"(SELECT * FROM get_route($1,$2);)", userId,routeId);
    try{
        auto result = f2.get();
        const auto& row = result[0];
        Json::Value item = getJsonItemByRow(req,row);
        retJsonValue.append(item); // Добавляем объект в массив
    }catch (...){
        LOG_ERROR_C<<"Unknown error in database";
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }
    resp->setStatusCode(k200OK);
    resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
    callback(resp);
}

void Routes::deleteRoute(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId){
    auto resp=HttpResponse::newHttpResponse();
    if(!userIsExists(resp,req,userId)){
        callback(resp);
        return;
    }
    if(!routeIsExists(resp,req,userId,routeId)){
        callback(resp);
        return;
    }
    auto f = dbClient->execSqlAsyncFuture(
            R"(DELETE FROM route_monitoring WHERE users_id =$1 AND route_monitoring_id=$2;)", userId,routeId);
    try{
        auto result = f.get();
        LOG_DEBUG_C<<fmt::format("Route id = {} deleted",routeId);
        resp->setStatusCode(k200OK);
    }catch (...){
        LOG_ERROR_C<<"Unknown error in database";
        resp->setStatusCode(k500InternalServerError);
    }
    callback(resp);
}