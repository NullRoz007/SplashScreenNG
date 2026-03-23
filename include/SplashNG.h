#pragma once
#define OEMRESOURCE

#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <winuser.h>
#include <wrl/client.h>
#include <shellapi.h>

#include <atomic>
#include <fstream>
#include <thread>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

using namespace std;
using namespace Microsoft::WRL;
using namespace SKSE;

namespace SplashNG {
    class SplashCursor;

    const static float FALLBACK_FONT_PADDING = 5.0f;
    const static int FALLBACK_FONT_SIZE = 12;
    const static int FALLBACK_WIDTH = 835;
    const static int FALLBACK_HEIGHT = 400;
    const static int FALLBACK_SPINNER_WIDTH = 32;
    const static int FALLBACK_SPINNER_HEIGHT = 32;

    const static wstring FALLBACK_SPLASH = L"splash.png";
    const static wstring FALLBACK_SPINNER = L"spinner.gif";
    const static wstring FALLBACK_FONT = L"Consolas";
    
    static const wchar_t CLASS_NAME[] = L"SplashScreenNG";
    constexpr UINT WM_SPLASH_CLOSE = WM_APP + 1;
    
    class Splash {
    public:
        static Splash& GetInstance() {
            static Splash instance;
            return instance;
        }

        static void ShowSplash();
        static void CloseSplash();

    private:
        static Splash* gSplash;
        void ShowSplashImpl();
        void CloseSplashImpl();

        HWND hSplash;
        wstring splashFile;
        wstring spinnerFile;
        vector<uint8_t> backgroundPixels;
        int width;
        int height;
        bool _isClosing;
        bool useText;
        int textX;
        int textY;
        wstring font;
        int textSize;
        int textColorR;
        int textColorG;
        int textColorB;
        float textPadding;
        DWRITE_TEXT_ALIGNMENT textAlign;
        DWRITE_PARAGRAPH_ALIGNMENT paraAlign;
        DWRITE_FONT_WEIGHT textWeight;
        DWRITE_FONT_STYLE textStyle;

        float targetOpacity;
        float currentOpacity;
        bool fadeIn;
        bool fadeOut;
        float fadeStep;

        bool useSpinner;
        int spinnerWidth;
        int spinnerHeight;
        int spinnerX;
        int spinnerY;
        int framesElapsed;
        bool draggable;
        bool fullscreen;
        bool forceFocus;

        bool useCursor;
        float cursorScale;

        string windowStyleName;
        ComPtr<ID2D1Factory> pD2DFactory;
        ComPtr<ID2D1DCRenderTarget> pRenderTarget;
        ComPtr<ID2D1SolidColorBrush> pBrush;
        ComPtr<IDWriteFactory> pIDWriteFactory;
        ComPtr<IDWriteTextFormat> pIDWriteTextFormat;
        ComPtr<ID2D1Bitmap> pBackgroundBitmap;
        std::vector<ComPtr<ID2D1Bitmap>>* pSpinnerFrames;
        SplashCursor* cursor;
        HINSTANCE hModule;
        thread sThread;
        string sThreadId;
        atomic_bool sRunning;

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        void ThreadEntry();

        HRESULT InitializeD2D();
        void CleanUpD2D();
        void RenderFrame();

        string SKSELastLogLine;
        string SKSELogPath;
        ifstream sSKSELogFile;
        HRESULT InitializeSKSELog();
        void UpdateSKSELogLine();

        void SetupConfig();
        void CreateSplashWindow();
    };
}