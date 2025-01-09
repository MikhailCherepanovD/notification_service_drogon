#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class User : public drogon::HttpController<User>
{
  public:
    METHOD_LIST_BEGIN
        METHOD_ADD(User::login, "/login", Get);
    METHOD_LIST_END

    void login(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);

};
