#pragma once
#include <drogon/HttpController.h>
#include <fmt/core.h>
#include "controllers_utils.h"
#include "aviasales_api/AviasalesApiRequester.h"
using namespace std;
using namespace drogon;
using namespace drogon::orm;

class Routes : public drogon::HttpController<Routes>
{
    shared_ptr<DbClient> dbClient;
    Json::StreamWriterBuilder singletonJsonWriter;
    AviasalesApiRequester& aviasalesApiRequester;
    set<string> allowedTypesOfJorney={"avia"};
    set<string> allowedTypesOfFrequencyOfMonitoring={"hours","minutes"};
    string routesUrl="users/{}/routes/{}";
    const int HASH_INTERVAL_TIME_MINUTES=30;  // потом вынести в конфиг файл
    bool checkIsCorrectJsonFieldForRoute( unordered_map<string,string>& jsonDict,shared_ptr<HttpRequest>req, shared_ptr<HttpResponse>resp,
                                                                         tm& dateBegin,tm& dateEnd);
    bool userIsExists(shared_ptr<HttpResponse>resp,shared_ptr<HttpRequest>req, string userId);
    bool routeIsExists(shared_ptr<HttpResponse>resp,shared_ptr<HttpRequest>req, string userId,string routeId);
    shared_ptr<HttpResponse> postOrPutRoute(const HttpRequestPtr &req, string& userId, string*  routeIdPtr=nullptr);
    Json::Value getJsonItemByRow(const HttpRequestPtr&,const Row&);
public:
    Routes(): dbClient{app().getDbClient("default") }, aviasalesApiRequester{AviasalesApiRequester::getInstance()}
    {}
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(Routes::postRoute, "users/{userId}/routes", Post);
        ADD_METHOD_TO(Routes::getRoutes, "users/{userId}/routes", Get);
        ADD_METHOD_TO(Routes::putRoute, "users/{userId}/routes/{routeId}", Put);
        ADD_METHOD_TO(Routes::getRoute, "users/{userId}/routes/{routeId}", Get);
        ADD_METHOD_TO(Routes::deleteRoute, "users/{userId}/routes/{routeId}", Delete);
        ADD_METHOD_TO(Routes::getCurrentDataRoute, "users/{user_id}/routes/{route_id}/current", Get);
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

};

