#pragma once 

#include <windows.h>
#include "nlohmann/json.hpp"
#include <fstream>

using json = nlohmann::json;
using namespace std;
namespace SplashNG {
    class Config {
    public:
        template <typename T>
        static T get(const std::string& name, const T& fallbackValue = T{});
        
        template <typename T>
        static T getFrom(const std::string& from, const std::string& name, const T& fallbackValue = T{});
        static void Initialize();
        static bool IsInitialized();
    private:
        static json j;
    };
}