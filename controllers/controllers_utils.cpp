#include "controllers_utils.h"
//help function:

bool checkIsCorrectJson(const HttpRequestPtr &req, shared_ptr<HttpResponse> resp){
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

string getInfoRequest(const HttpRequestPtr &req){
    return "During request: "+to_string(req->getMethod()) + req->getPath();
}

void getResponseMissingRequiredFields(const HttpRequestPtr &req, shared_ptr<HttpResponse> resp){
    LOG_DEBUG_C<<"Request error: Missing required fields";
    resp->setStatusCode(k400BadRequest);
    resp->setBody(R"({"error": "Missing required fields"})");
}


bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int getDaysInMonth(int month, int year) {
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days = daysInMonth[month - 1];
    if (month == 2 && isLeapYear(year)) {
        days = 29;
    }
    return days;
}

tm parseDate(const string& dateStr, int& status) {
    tm date = {};
    istringstream ss(dateStr);
    char delimiter;

    ss >> get_time(&date, "%Y-%m-%d");
    if (ss.fail()) {
        status = 0;
        return {};
    }

    int year = date.tm_year + 1900;
    int month = date.tm_mon + 1;
    int day = date.tm_mday;

    if (day > getDaysInMonth(month, year)) {
        status = 0;
        return {};
    }

    status = 1;
    return date;
}