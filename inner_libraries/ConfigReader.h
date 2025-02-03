#pragma once
#include <json/value.h>
#include <fstream>
#include "inner_libraries/Singleton.h"
using namespace std;
class ConfigReader: public Singleton<ConfigReader>{
    Json::Value config;
    const Json::Value* configPtr;
public:
    ConfigReader(){
        std::ifstream configFile("../config.json", std::ifstream::binary);

        if (!configFile.is_open()) {
            std::cerr << "Error opening config.json" << std::endl;
            return ;
        }
        // Читаем файл в переменную config
        configFile >> config;
        configPtr = &config;
    }
    const Json::Value* getJsonValue(){
        return configPtr;
    }

};