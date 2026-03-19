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
    bool forceFocus = Config::get<bool>("forceFocus", false);
    uint32_t kCloseEvent = Config::get<int>("closeOn", 6);

    if (message->type == MessagingInterface::kInputLoaded && forceFocus) {
        
        HWND hwndSkyrim = FindWindow(nullptr, L"Skyrim Special Edition");
        if (hwndSkyrim == 0) hwndSkyrim = FindWindow(nullptr, L"Skyrim Anniversary Edition");
        if (hwndSkyrim != 0) {
            log::info("Forcing focus to HWND {:#x}", reinterpret_cast<uintptr_t>(hwndSkyrim));

            DWORD foregroundThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
            DWORD skyrimThread = GetWindowThreadProcessId(hwndSkyrim, nullptr);
            
            AttachThreadInput(GetCurrentThreadId(), skyrimThread, TRUE);
            SetForegroundWindow(hwndSkyrim);
            SetFocus(hwndSkyrim);
            SetActiveWindow(hwndSkyrim);
            AttachThreadInput(GetCurrentThreadId(), skyrimThread, FALSE);

            SetWindowPos(hwndSkyrim, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowPos(hwndSkyrim, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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