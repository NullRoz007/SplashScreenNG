#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <wrl/client.h>
#include <shellscalingapi.h>

#include <atomic>
#include <fstream>
#include <thread>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

using namespace std;
using namespace Microsoft::WRL;
using namespace SKSE;

namespace SplashNG {
    const static float FALLBACK_FONT_PADDING = 5.0f;
    const static int FALLBACK_FONT_SIZE = 12;
    const static int FALLBACK_WIDTH = 835;
    const static int FALLBACK_HEIGHT = 400;
    const static int FALLBACK_SPINNER_WIDTH = 32;
    const static int FALLBACK_SPINNER_HEIGHT = 32;

    const static wstring FALLBACK_SPLASH = L"splash.png";
    const static wstring FALLBACK_SPINNER = L"spinner.gif";
    const static wstring FALLBACK_FONT = L"Consolas";
    
    static const wchar_t CLASS_NAME[] = L"Splash Screen NG";
    constexpr UINT WM_SPLASH_CLOSE = WM_APP + 1;
    
    class Splash {
    public:
        static HWND hSplash;
        static void ShowSplash();
        static void CloseSplash();

    private:
        static wstring splashFile;
        static wstring spinnerFile;

        static int width;
        static int height;
        static bool _isClosing;

        static bool useText;
        static int textX;
        static int textY;
        static wstring font;
        static int textSize;
        static int textColorR;
        static int textColorG;
        static int textColorB;
        static float textPadding;
        static DWRITE_TEXT_ALIGNMENT textAlign;
        static DWRITE_PARAGRAPH_ALIGNMENT paraAlign;
        static DWRITE_FONT_WEIGHT textWeight;
        static DWRITE_FONT_STYLE textStyle;


        static float targetOpacity;
        static float currentOpacity;
        static bool fadeIn;
        static bool fadeOut;
        static float fadeStep;

        static bool useSpinner;
        static int spinnerWidth;
        static int spinnerHeight;
        static int spinnerX;
        static int spinnerY;
        static int framesElapsed;
        static bool draggable;

        static ComPtr<ID2D1Factory> pD2DFactory;
        static ComPtr<ID2D1DCRenderTarget> pRenderTarget;
        static ComPtr<ID2D1SolidColorBrush> pBrush;
        static ComPtr<IDWriteFactory> pIDWriteFactory;
        static ComPtr<IDWriteTextFormat> pIDWriteTextFormat;
        static ComPtr<ID2D1Bitmap> pBackgroundBitmap;
        static std::vector<ComPtr<ID2D1Bitmap>>* pSpinnerFrames;

        static HINSTANCE hModule;
        static thread sThread;
        static string sThreadId;
        static atomic_bool sRunning;

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void ThreadEntry();

        static HRESULT InitializeD2D();
        static void CleanUpD2D();
        static void RenderFrame();

        static string SKSELastLogLine;
        static string SKSELogPath;
        static ifstream sSKSELogFile;
        static HRESULT InitializeSKSELog();
        static void UpdateSKSELogLine();

        static void SetupConfig();
        static void CreateSplashWindow();
    };
}