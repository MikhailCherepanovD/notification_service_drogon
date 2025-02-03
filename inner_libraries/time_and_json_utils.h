#pragma once
#include <drogon/HttpController.h>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <iomanip>
using namespace std;
using namespace drogon;
using namespace drogon::orm;
#define LOG_DEBUG_C LOG_DEBUG<<" "<<getInfoRequest(req)
#define LOG_WARN_C LOG_WARN<<" "<<getInfoRequest(req)
#define LOG_ERROR_C LOG_ERROR<<" "<<getInfoRequest(req)
//Requests
bool checkIsCorrectJson(const HttpRequestPtr &req,shared_ptr<HttpResponse> resp);
string getInfoRequest(const HttpRequestPtr &req);
bool tryParseJson(const std::string& jsonStr, Json::Value& jsonData);
//Time
void getResponseMissingRequiredFields(const HttpRequestPtr &req, shared_ptr<HttpResponse> resp);
tm parseDate(const string& dateStr, int& status);
string getCurrentDateAndTime();
string fromDateToStr(const tm& date);
vector<string> getDatesBetween(const tm& dateBegin, const tm& dateEnd);
//Other
string boolToStr(bool value);