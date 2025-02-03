#pragma once
#include <drogon/HttpController.h>
#include <fmt/core.h>
#include "inner_libraries/time_and_json_utils.h"
#include "aviasales_api/AviasalesApiRequester.h"
#include "inner_libraries/ConfigReader.h"
using namespace std;
using namespace drogon;
using namespace drogon::orm;

class Routes : public drogon::HttpController<Routes>
{
    //Fields
    shared_ptr<DbClient> dbClient;
    Json::StreamWriterBuilder singletonJsonWriter;
    AviasalesApiRequester& aviasalesApiRequester;
    ConfigReader &configReader;

    set<string> allowedTypesOfJorney={"avia"};
    set<string> allowedTypesOfFrequencyOfMonitoring={"hours","minutes"};
    string routesUrl="users/{}/routes/{}";
    int hash_interval_time_minutes;
    //Help methods
    bool checkIsCorrectJsonFieldForRoute( unordered_map<string,string>& jsonDict,shared_ptr<HttpRequest>req, shared_ptr<HttpResponse>resp,
                                                                         tm& dateBegin,tm& dateEnd);
    bool userIsExists(shared_ptr<HttpResponse>resp,shared_ptr<HttpRequest>req, string userId);
    bool routeIsExists(shared_ptr<HttpResponse>resp,shared_ptr<HttpRequest>req, string userId,string routeId);
    shared_ptr<HttpResponse> postOrPutRoute(const HttpRequestPtr &req, string& userId, string*  routeIdPtr=nullptr);
    Json::Value getJsonItemByRow(const HttpRequestPtr&,const Row&);
    unordered_map<string,string> getJsonDictByJourney(shared_ptr<Json::Value> jo);
public:
    Routes():
    dbClient{app().getDbClient("default") },
    aviasalesApiRequester{AviasalesApiRequester::getInstance()},
    configReader{ConfigReader::getInstance()}{
        hash_interval_time_minutes = (*configReader.getJsonValue())["hash_interval_time_minutes"].asInt();
    }
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(Routes::postRoute, "users/{userId}/routes", Post);
        ADD_METHOD_TO(Routes::getRoutes, "users/{userId}/routes", Get);
        ADD_METHOD_TO(Routes::putRoute, "users/{userId}/routes/{routeId}", Put);
        ADD_METHOD_TO(Routes::getRoute, "users/{userId}/routes/{routeId}", Get);
        ADD_METHOD_TO(Routes::deleteRoute, "users/{userId}/routes/{routeId}", Delete);
        ADD_METHOD_TO(Routes::getCurrentDataRoute, "users/{user_id}/routes/{route_id}/current", Get);
        ADD_METHOD_TO(Routes::getCheapestDataRoute, "users/{user_id}/routes/{route_id}/cheapest", Get);
        ADD_METHOD_TO(Routes::getStatisticDataRoute, "users/{user_id}/routes/{route_id}/statistic", Get);
    METHOD_LIST_END
    void postRoute(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback, string &&userId);
    void getRoutes(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback, string &&userId);
    void putRoute(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId);
    void getRoute(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId);
    void deleteRoute(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId);
    void getCurrentDataRoute(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId);
    void getCheapestDataRoute(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId);
    void getStatisticDataRoute(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback, string &&userId, string &&routeId);

};

