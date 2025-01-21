#pragma once
#include <drogon/HttpController.h>
#include <fmt/core.h>
using namespace std;
using namespace drogon;
using namespace drogon::orm;
#define LOG_DEBUG_C LOG_DEBUG<<getInfoRequest(req)
#define LOG_WARN_C LOG_WARN<<getInfoRequest(req)
#define LOG_ERROR_C LOG_ERROR<<getInfoRequest(req)
class User : public drogon::HttpController<User>
{
    shared_ptr<DbClient> dbClient;
    Json::StreamWriterBuilder singletonJsonWriter;

    bool checkIsCorrectJson(const HttpRequestPtr &req,shared_ptr<HttpResponse> resp);
    string getInfoRequest(const HttpRequestPtr &req);
    void getResponseMissingRequiredFields(const HttpRequestPtr &req, shared_ptr<HttpResponse> resp);
public:
    User():dbClient{ app().getDbClient("default") }{}
    METHOD_LIST_BEGIN
        METHOD_ADD(User::login, "/login", Post);
        METHOD_ADD(User::registration, "/register", Post);
        METHOD_ADD(User::updateWholeInfo,"/{id}",Put);
        METHOD_ADD(User::getWholeInfo,"/{id}",Get);
        METHOD_ADD(User::deleteUser,"/{id}",Delete);
        METHOD_ADD(User::testPoint, "/testPoint", Post);
    METHOD_LIST_END

    void login(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
    void registration(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
    void updateWholeInfo(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback, string &&id);
    void getWholeInfo(const HttpRequestPtr &req,
                         std::function<void(const HttpResponsePtr &)> &&callback, string &&id);
    void deleteUser(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback, string &&id);

    void testPoint(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback);

};
