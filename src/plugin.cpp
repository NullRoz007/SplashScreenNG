#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <SplashNG.h>
#include <Config.h>

using namespace SKSE;
using namespace SKSE::stl;
using namespace SplashNG;

void OnMessage(MessagingInterface::Message* message) {
    uint32_t kCloseEvent = Config::get<int>("closeOn", 6);
    if (message->type == MessagingInterface::kInputLoaded) {
        HWND hwndSkyrim = FindWindow(nullptr, L"Skyrim Special Edition");
        if (hwndSkyrim == 0) hwndSkyrim = FindWindow(nullptr, L"Skyrim Anniversary Edition");

        if (hwndSkyrim != 0) {
            HICON hIcon = nullptr;
            hIcon = (HICON)SendMessage(Splash::hSplash, WM_GETICON, ICON_BIG, 0);
            if (hIcon) {
                SendMessage(hwndSkyrim, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
                SendMessage(hwndSkyrim, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
                SetClassLongPtr(hwndSkyrim, GCLP_HICON, (LONG_PTR)hIcon);
                SetClassLongPtr(hwndSkyrim, GCLP_HICONSM, (LONG_PTR)hIcon);
            }
        }
    }

    if (message->type == kCloseEvent) { // 6 = kInputLoaded, 8 = kDataLoaded
        Splash::CloseSplash();
    }
}

void InitLogging() {
    auto path = log::log_directory();
    spdlog::level::level_enum level;

    if (!path) report_and_fail("Unable to locate SKSE logs directory!");

    *path /= PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    std::shared_ptr<spdlog::logger> log;
    
    if (IsDebuggerPresent()) {
        log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        level = spdlog::level::trace;
    } else {
        log = std::make_shared<spdlog::logger>(
            "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        level = spdlog::level::info;
    }

    log->set_level(level);
    log->flush_on(level);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    InitLogging();
    std::string tId = std::format("{}", std::this_thread::get_id());
    log::info("Main Thread: {}", tId);
    
    Config::Initialize();
    Splash::ShowSplash();

    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);

    return true;
}