#pragma once
#include <drogon/HttpController.h>
#include <fmt/core.h>
#include "inner_libraries/time_and_json_utils.h"
using namespace std;
using namespace drogon;
using namespace drogon::orm;

class Users : public drogon::HttpController<Users>
{
    shared_ptr<DbClient> dbClient;
    Json::StreamWriterBuilder singletonJsonWriter;
    string usersUrl="users/{}";
public:
    Users(): dbClient{app().getDbClient("default") }{}
    METHOD_LIST_BEGIN
        METHOD_ADD(Users::login, "/login", Post);
        METHOD_ADD(Users::registration, "/register", Post);
        METHOD_ADD(Users::updateWholeInfo, "/{id}", Put);
        METHOD_ADD(Users::getWholeInfo, "/{id}", Get);
        METHOD_ADD(Users::deleteUser, "/{id}", Delete);
        METHOD_ADD(Users::testPoint, "/testPoint", Post);
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
