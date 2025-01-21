#include "User.h"

//help function:
bool User::checkIsCorrectJson(const HttpRequestPtr &req,shared_ptr<HttpResponse> resp){
    if(req->getHeader("Content-Type")!="application/json"){
        resp->setStatusCode(k415UnsupportedMediaType);
        resp->setBody(R"({"error": "Expected JSON content"})");
        LOG_WARN_C<<"Unsupported media type of request";
        return 0;
    }
    if(!req->getJsonObject()){
        resp->setStatusCode(k400BadRequest);
        resp->setBody(R"({"error": "Missing required fields"})");
        LOG_DEBUG_C<<"Reuest error: Missing required fields";
        return 0;
    }
    return 1;
}

string User::getInfoRequest(const HttpRequestPtr &req){
    return "During request: "+to_string(req->getMethod()) + req->getPath();
}

void User::getResponseMissingRequiredFields(const HttpRequestPtr &req, shared_ptr<HttpResponse> resp){
    LOG_DEBUG_C<<"Request error: Missing required fields";
    resp->setStatusCode(k400BadRequest);
    resp->setBody(R"({"error": "Missing required fields"})");
}
// Handlers:
void User::login(const HttpRequestPtr &req,
                 function<void (const HttpResponsePtr &)> &&callback)
{
    Json::Value retJsonValue;
    auto resp=HttpResponse::newHttpResponse();
    if(!checkIsCorrectJson(req,resp)){
        callback(resp);
        return;
    }
    auto jo =req->getJsonObject();
    string login = (*jo)["login"].asString();
    string password = (*jo)["password"].asString();
    if(login=="" || password==""){
        getResponseMissingRequiredFields(req,resp);
        callback(resp);
        return;
    }
    auto f = dbClient->execSqlAsyncFuture(R"(
       SELECT users_id, login, user_name, email, telegram FROM users WHERE login = $1 AND password=$2)",login,password);
    try{
        auto result=f.get();
        if(result.affectedRows()==0){
            LOG_DEBUG_C<<fmt::format("User with login {} is not exist", login);
            retJsonValue["id"] = Json::nullValue;
            retJsonValue["message"]="login or password is incorrect";
            resp->setStatusCode(k401Unauthorized);
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        }
        else if(result.affectedRows()==1){
            long users_id = result[0]["users_id"].as<long>();
            // users_id, login, user_name, email, telegram
            LOG_DEBUG_C<<fmt::format("User with login: {} and  id: {}", login,users_id);
            retJsonValue["id"] = users_id;
            retJsonValue["login"] = result[0]["login"].as<string>();
            retJsonValue["user_name"] = result[0]["user_name"].as<string>();
            retJsonValue["email"] = result[0]["email"].as<string>();
            retJsonValue["telegram"] = result[0]["telegram"].as<string>();
            resp->setStatusCode(k200OK);
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        }
        else{
            LOG_WARN_C<<fmt::format("Error of DB: several users with same login: {}  and password: {}",login,password);
            retJsonValue["id"] = Json::nullValue;
            resp->setStatusCode(k500InternalServerError);
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        }
    }catch(const DrogonDbException &e){
        LOG_WARN_C<<fmt::format("Error of DB with login: {}, password: {} ",login,password)<<e.base().what();
        retJsonValue["id"] = Json::nullValue;
        resp->setStatusCode(k500InternalServerError);
        resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
    }

    callback(resp);
}


void User::registration(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback){
    auto resp=HttpResponse::newHttpResponse();
    if(!checkIsCorrectJson(req,resp)){
        callback(resp);
        return;
    }

    auto jo =req->getJsonObject();
    string login = (*jo)["login"].asString();
    string password = (*jo)["password"].asString();
    string user_name = (*jo)["user_name"].asString();
    string email = (*jo)["email"].asString();
    string telegram = (*jo)["telegram"].asString();

    if(login=="" || password=="" || user_name==""){
        resp->setStatusCode(k400BadRequest);
        resp->setBody(R"({"error": "Missing required fields"})");
        callback(resp);
        return;
    }
    auto f = dbClient->execSqlAsyncFuture(R"(
        INSERT INTO users (login, password, user_name, email, telegram)
        VALUES($1, $2, $3, $4, $5))",login,password,user_name,email,telegram);
    try{
        auto result=f.get();
        LOG_DEBUG_C<<fmt::format("User with login: {} was created", login);
        resp->setStatusCode(k201Created);
    }catch(const DrogonDbException &e){
        LOG_WARN_C<<fmt::format("User with login: {} already exists",login)<<e.base().what();
        resp->setStatusCode(k409Conflict);
        resp->setBody(R"({"error": "User already exists"})");
    }
    callback(resp);
}

void User::updateWholeInfo(const HttpRequestPtr &req,
                           std::function<void(const HttpResponsePtr &)> &&callback, string &&userId){
    auto resp=HttpResponse::newHttpResponse();
    if(!checkIsCorrectJson(req,resp)){
        callback(resp);
        return;
    }
    auto jo =req->getJsonObject();
    string login = (*jo)["login"].asString();
    string password = (*jo)["password"].asString();
    string userName = (*jo)["user_name"].asString();
    string email = (*jo)["email"].asString();
    string telegram = (*jo)["telegram"].asString();

    if(login=="" || password=="" || userName == ""){
        resp->setStatusCode(k400BadRequest);
        resp->setBody(R"({"error": "Missing required fields"})");
        callback(resp);
        return;
    }

    auto f = dbClient->execSqlAsyncFuture(R"(SELECT update_or_insert_users($1,$2,$3,$4,$5,$6)	)",
                                          userId, login, password, userName, email, telegram);
    try{
        int statusUserCreating = f.get().at(0)["update_or_insert_users"].as<int>();
        if(statusUserCreating==1) {
            LOG_DEBUG_C << fmt::format("User with login: {} was created", login);
            resp->setStatusCode(k201Created);
        }
        else if(statusUserCreating==2){
            LOG_DEBUG_C << fmt::format("User with login: {} was updated", login);
            resp->setStatusCode(k200OK);
        }
        else if(statusUserCreating==0){
            LOG_DEBUG_C << fmt::format("User with login: {} already exists", login);
            resp->setStatusCode(k409Conflict);
        }else{
            LOG_WARN_C << fmt::format("Unknown answer from DB for login: {} ", login);
            resp->setStatusCode(k500InternalServerError);
        }
    }catch(const DrogonDbException &e){
        LOG_WARN_C << fmt::format("Unknown DB error for login: {} ", login);
        resp->setStatusCode(k500InternalServerError);
    }
    callback(resp);
}



void User::getWholeInfo(const HttpRequestPtr &req,
                           std::function<void(const HttpResponsePtr &)> &&callback, string &&userId){
    auto resp=HttpResponse::newHttpResponse();
    Json::Value retJsonValue;

    auto f = dbClient->execSqlAsyncFuture(R"(SELECT * FROM users WHERE users_id=$1)",userId);
    try{
        auto result=f.get();
        int amountRows = result.affectedRows();
        if(result.affectedRows()==0){
            LOG_DEBUG_C << fmt::format("User with id: {} do not exists", userId);
            resp->setStatusCode(k404NotFound);
        }else{
            retJsonValue["users_id"] = result[0]["users_id"].as<string>();
            retJsonValue["login"] = result[0]["login"].as<string>();
            retJsonValue["user_name"] = result[0]["user_name"].as<string>();
            retJsonValue["email"] = result[0]["email"].as<string>();
            retJsonValue["telegram"] = result[0]["telegram"].as<string>();
            resp->setStatusCode(k200OK);
            resp->setBody(Json::writeString(singletonJsonWriter, retJsonValue));
        }
    }catch(const DrogonDbException &e){
        LOG_WARN_C << fmt::format("Unknown DB error for userId: {} ", userId);
        resp->setStatusCode(k500InternalServerError);
    }
    callback(resp);
}


void User::deleteUser(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback, string &&userId){
    auto resp=HttpResponse::newHttpResponse();
    auto f = dbClient->execSqlAsyncFuture(R"(DELETE FROM users WHERE users_id=$1)",userId);
    try{
        auto result=f.get();
        int amountRows = result.affectedRows();
        if(result.affectedRows()==0){
            LOG_DEBUG_C << fmt::format("User with id: {} do not exists", userId);
            resp->setStatusCode(k404NotFound);
        }else{
            resp->setStatusCode(k200OK);
        }
    }catch(const DrogonDbException &e){
        LOG_WARN_C << fmt::format("Unknown DB error for userId: {} ", userId);
        resp->setStatusCode(k500InternalServerError);
    }
    callback(resp);
}




void User::testPoint(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback){
    Json::Value ret;
    HttpMethod method =req->getMethod();
    string header1 = req->getHeader("header1");
    string header2 = req->getHeader("header2");
    SafeStringMap<string> smp = req->getHeaders();
    SafeStringMap<string> cookies = req->getCookies();
    long contentLength = req->getRealContentLength();
    long bodyLength = req->bodyLength();
    string_view str_view = req->getBody();
    Version ver =  req->getVersion();
    shared_ptr<HttpResponse> resp = HttpResponse::newHttpResponse(k201Created,CT_APPLICATION_JSON);
    auto jo =req->getJsonObject();
    string login = (*jo)["login"].asString();
    string password = (*jo)["password"].asString();
    string user_name = (*jo)["user_name"].asString();
    string email=(*jo)["email"].asString();
    string telegram=(*jo)["telegram"].asString();
    Json::Value jsonResponse;
    jsonResponse["login"] = "login";
    jsonResponse["message"] = "Resource created successfully";

    string jsonString = Json::writeString(singletonJsonWriter, jsonResponse);
    resp->setBody(jsonString);

    try
    {
        auto result = dbClient->execSqlSync(R"(SELECT * FROM users)"); // Block until we get the result or catch the exception;
        LOG_DEBUG_C << result.affectedRows() << " rows updated!";
    }
    catch (const DrogonDbException &e)
    {
        LOG_ERROR_C  << "error:" << e.base().what();
    }
    callback(resp);

}
