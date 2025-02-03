#include "time_and_json_utils.h"


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
bool tryParseJson(const std::string& jsonStr, Json::Value& jsonData) {
    Json::CharReaderBuilder readerBuilder;
    std::istringstream s(jsonStr);
    std::string errs;
    return Json::parseFromStream(readerBuilder, s, &jsonData, &errs);
}


//Time
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


string getCurrentDateAndTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Преобразуем в структуру tm
    std::tm local_tm = *std::localtime(&now_time);

    // Форматируем время в строку
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

string fromDateToStr(const tm& date) {
    ostringstream oss;
    oss << (date.tm_year + 1900) << "-"
        << std::setw(2) << std::setfill('0') << (date.tm_mon + 1) << "-"
        << std::setw(2) << std::setfill('0') << date.tm_mday;
    return oss.str();
}

vector<string> getDatesBetween(const tm& dateBegin, const tm& dateEnd) {
    vector<string> dates;
    tm current = dateBegin;
    // Преобразуем даты в time_t для работы с днями
    time_t timeBegin = mktime(const_cast<tm*>(&dateBegin));
    time_t timeEnd = mktime(const_cast<tm*>(&dateEnd));
    if (timeBegin == -1 || timeEnd == -1 || timeBegin > timeEnd) {
        return dates; // Если данные некорректны или конец меньше начала, возвращаем пустой вектор
    }
    while (difftime(timeEnd, timeBegin) >= 0) {
        // Добавляем текущую дату в вектор
        dates.push_back(fromDateToStr(current));

        // Увеличиваем день на 1
        current.tm_mday++;
        timeBegin = mktime(&current);
    }
    return dates;
}
//Other

string boolToStr(bool value) {
    return value == 1 ? "true" : "false";
}
