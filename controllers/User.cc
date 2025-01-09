#include "User.h"
using namespace std;
using namespace drogon::orm;

void User::login(const HttpRequestPtr &req,
                 std::function<void (const HttpResponsePtr &)> &&callback)
{
    Json::Value ret;
    std::string connInfo = "host=127.0.0.1 port=5432 dbname=postgres user=postgres password=7052018";
    int val=10;
    auto dbClient = app().getDbClient("default");
    auto f= dbClient->execSqlAsyncFuture("select * from passenger where passenger_id=$1",1);
    try{
        auto result=f.get();
        LOG_DEBUG<<"was "<<result.affectedRows()<<"rows selected";
        for(auto row:result){
            ret[row["login"].as<string>()]=row["password"].as<string>();
        }
    }catch(const DrogonDbException &e){
        LOG_ERROR<<"error selecting from database: "<<e.base().what();
    }

    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}