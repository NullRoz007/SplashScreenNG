#include <sstream>
#include <SplashNG.h>
#include <SplashBitmap.h>
#include <Config.h>

using namespace SKSE;

namespace SplashNG {
    atomic_bool Splash::sRunning{false};
    HINSTANCE Splash::hModule;
    HWND Splash::hSplash;

    thread Splash::sThread;
    string Splash::sThreadId;

    ComPtr<ID2D1Factory> Splash::pD2DFactory;
    ComPtr<ID2D1DCRenderTarget> Splash::pRenderTarget;
    ComPtr<ID2D1SolidColorBrush> Splash::pBrush;
    ComPtr<IDWriteFactory> Splash::pIDWriteFactory;
    ComPtr<IDWriteTextFormat> Splash::pIDWriteTextFormat;
    ComPtr<ID2D1Bitmap> Splash::pBackgroundBitmap;
    vector<ComPtr<ID2D1Bitmap>>* Splash::pSpinnerFrames;

    ifstream Splash::sSKSELogFile;
    string Splash::SKSELastLogLine;
    string Splash::SKSELogPath;

    int Splash::height;
    int Splash::width;
    bool Splash::draggable;
    bool Splash::_isClosing;
    wstring Splash::splashFile;
    wstring Splash::spinnerFile;

    int Splash::textX;
    int Splash::textY;
    DWRITE_TEXT_ALIGNMENT Splash::textAlign;
    DWRITE_PARAGRAPH_ALIGNMENT Splash::paraAlign;

    wstring Splash::font;
    int Splash::fontSize;
    int Splash::fontColorR;
    int Splash::fontColorG;
    int Splash::fontColorB;
    float Splash::fontPadding;

    float Splash::currentOpacity;
    float Splash::targetOpacity;
    float Splash::fadeStep;
    bool Splash::fadeIn;
    bool Splash::fadeOut;

    int Splash::spinnerWidth;
    int Splash::spinnerHeight;
    int Splash::spinnerX;
    int Splash::spinnerY;

    bool Splash::useText;
    bool Splash::useSpinner;
    int Splash::framesElapsed = 0;

    struct SplashTextAlignment {
        DWRITE_TEXT_ALIGNMENT textAlignment;
        DWRITE_PARAGRAPH_ALIGNMENT paraAlignment;
    };

    SplashTextAlignment getTextAlignment() {
        SplashTextAlignment result = {};

        string textAlignName = Config::get<string>("textAlignment", "center");
        result.textAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
        result.paraAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

        try {
            result.textAlignment = static_cast<DWRITE_TEXT_ALIGNMENT>(Config::getFrom<int>("textAlignments", textAlignName, 2));
            switch (result.textAlignment) {
                case DWRITE_TEXT_ALIGNMENT_LEADING:
                    result.paraAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
                    break;
                case DWRITE_TEXT_ALIGNMENT_CENTER:
                    result.paraAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
                    break;
                case DWRITE_TEXT_ALIGNMENT_TRAILING:
                    result.paraAlignment = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
                    break;
            }
        } catch (exception& ex) {
            log::error("Invalid value passed to textAlign {}", ex.what());
        }

        return result;
    }

    LRESULT CALLBACK Splash::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_SPLASH_CLOSE:
                if (hSplash) {
                    if (fadeOut) {
                        useText = false; //easier than fading the text
                        _isClosing = true;
                    } else {
                        log::info("Destroying Splash");
                        log::info("Thread ID: {}", sThreadId);
                        sSKSELogFile.close();
                        DestroyWindow(hSplash);
                    }
                }

                return 0;

            case WM_SETCURSOR:
                SetCursor(LoadCursor(nullptr, IDC_ARROW));
                return TRUE;

            case WM_NCHITTEST:
                return Splash::draggable ? HTCAPTION : HTCLIENT;

            case WM_MOUSEACTIVATE:
                return MA_NOACTIVATE;
           
            case WM_DESTROY:
                log::info("Posting Quit Message");
                log::info("Thread ID: {}", sThreadId);
                PostQuitMessage(0);
                sRunning = false;
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void Splash::CleanUpD2D() {
        pBackgroundBitmap.Reset();
        pIDWriteTextFormat.Reset();
        pIDWriteFactory.Reset();
        pBrush.Reset();
        pRenderTarget.Reset();
        pD2DFactory.Reset();

        if (pSpinnerFrames) {
            for (int i = 0; i < pSpinnerFrames->size(); i++) {
                pSpinnerFrames->at(i).Reset();
            }
        }
        
    }

    HRESULT Splash::InitializeD2D() {
        if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
            log::error("Failed to CoInitialize");
            return E_ABORT;
        }

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory.GetAddressOf());

        if (SUCCEEDED(hr)) {
            D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT, 
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM, 
                    D2D1_ALPHA_MODE_PREMULTIPLIED
                )
            );

            hr = pD2DFactory->CreateDCRenderTarget(&props, pRenderTarget.GetAddressOf());
        }

        if (SUCCEEDED(hr)) {
            hr = pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(RGB(
                   fontColorB,
                   fontColorG, 
                   fontColorR)
                ), 
                pBrush.GetAddressOf()
            );
        }

        if (SUCCEEDED(hr)) {
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED, 
                __uuidof(pIDWriteFactory),
                reinterpret_cast<IUnknown**>(pIDWriteFactory.GetAddressOf())
            );
        }

        if (SUCCEEDED(hr)) {
            hr = pIDWriteFactory->CreateTextFormat(
                font.c_str(), 
                nullptr, 
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL, 
                DWRITE_FONT_STRETCH_NORMAL, 
                fontSize,
                L"", 
                pIDWriteTextFormat.GetAddressOf()
            );
        }

        if (SUCCEEDED(hr)) {
            pIDWriteTextFormat->SetTextAlignment(textAlign); 
            pIDWriteTextFormat->SetParagraphAlignment(paraAlign);
            pIDWriteTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        }

        if (SUCCEEDED(hr)) {
            hr = CoCreateInstance(
                CLSID_WICImagingFactory, 
                nullptr, 
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(SplashBitmap::pIWICFactory.GetAddressOf())
            );
        }

        if (SUCCEEDED(hr)) {
            hr = SplashBitmap::LoadBitmapFromFile(
                pRenderTarget.Get(),
                splashFile.c_str(), 
                width,
                height,
                pBackgroundBitmap.GetAddressOf()
            ); 
        }

        if (useSpinner) {
            hr = SplashBitmap::GetFramesFromFile(pRenderTarget.Get(), spinnerFile.c_str(), spinnerWidth, spinnerHeight,
                                                 pSpinnerFrames);
            if (SUCCEEDED(hr)) {
                log::info("spinner frames: {}", pSpinnerFrames->size());
            }
        }

        if (FAILED(hr)) {
            log::error(
                "InitializeD2D failed (HRESULT {:#010x}) loading {}", 
                static_cast<unsigned>(hr), 
                std::string(splashFile.begin(), splashFile.end())
            );
        }

        return hr;
    }

    void Splash::RenderFrame() {
        HDC hdcScreen = GetDC(nullptr);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pvBits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(
            hdcScreen, 
            &bmi, 
            DIB_RGB_COLORS, 
            &pvBits, 
            nullptr, 
            0
        );
        
        HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hdcMem, hBitmap));

        RECT rc = {
            0, 
            0, 
            width, 
            height
        };
        
        pRenderTarget->BindDC(hdcMem, &rc);

        std::wstring wline(SKSELastLogLine.begin(), SKSELastLogLine.end());

        D2D1_SIZE_F sz = pRenderTarget->GetSize();
        D2D1_RECT_F bgRect = D2D1::RectF(
            0, 
            0, 
            sz.width, 
            sz.height
        );

        D2D1_RECT_F spRect = D2D1::RectF(
            spinnerX, 
            spinnerY, 
            spinnerWidth + spinnerX, 
            spinnerHeight + spinnerY
        );
        
        D2D1_RECT_F textRect = D2D1::RectF(
            textX + fontPadding, 
            textY + fontPadding, 
            sz.width - fontPadding, 
            sz.height - pIDWriteTextFormat->GetFontSize() - fontPadding
        );

        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

        if (pBackgroundBitmap) {
            pRenderTarget->DrawBitmap(
                pBackgroundBitmap.Get(), 
                bgRect, 
                currentOpacity, 
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
            );
        }

        if (useSpinner && pSpinnerFrames) {
            pRenderTarget->DrawBitmap(pSpinnerFrames->at(framesElapsed % pSpinnerFrames->size()).Get(), 
                spRect, 
                currentOpacity,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
            );
        }

        if (useText) {        
            pRenderTarget->DrawText(wline.c_str(),
                wline.length(), 
                pIDWriteTextFormat.Get(), 
                textRect,
                pBrush.Get()
            );
        }

        HRESULT hr = pRenderTarget->EndDraw();

        if (FAILED(hr)) {
            pRenderTarget.Reset();
        }

        POINT ptSrc = {0, 0};
        SIZE szWin = {width, height};
        POINT ptDest = {};
        RECT wndRc = {};
        GetWindowRect(hSplash, &wndRc);
        ptDest.x = wndRc.left;
        ptDest.y = wndRc.top;

        BLENDFUNCTION blend = {};
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        UpdateLayeredWindow(
            hSplash, 
            hdcScreen, 
            &ptDest, 
            &szWin, 
            hdcMem, 
            &ptSrc, 
            0, 
            &blend, 
            ULW_ALPHA
        );

        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
    }

    void Splash::SetupConfig() {
        Config::Initialize();

        width = Config::get<int>("width", FALLBACK_WIDTH);
        height = Config::get<int>("height", FALLBACK_HEIGHT);

        font = Config::get<wstring>("textFont", FALLBACK_FONT);
        fontSize = Config::get<int>("textFontSize", FALLBACK_FONT_SIZE);
        fontColorR = Config::get<int>("textColorR", 255);
        fontColorG = Config::get<int>("textColorG", 255);
        fontColorB = Config::get<int>("textColorB", 255);
        fontPadding = Config::get<float>("textPadding", FALLBACK_FONT_PADDING);

        fadeIn = Config::get<bool>("fadeIn", false);
        fadeOut = Config::get<bool>("fadeOut", false);
        fadeStep = Config::get<float>("fadeStep", 0.1);
        spinnerWidth = Config::get<int>("spinnerWidth", FALLBACK_SPINNER_WIDTH);
        spinnerHeight = Config::get<int>("spinnerHeight", FALLBACK_SPINNER_HEIGHT);
        spinnerX = Config::get<int>("spinnerX", 0);
        spinnerY = Config::get<int>("spinnerY", 0);
        targetOpacity = Config::get<float>("opacity", 1.0f);

        useSpinner = Config::get<bool>("useSpinner", false);
        useText = Config::get<bool>("useText", false);
        draggable = Config::get<bool>("draggable", true);

        textX = Config::get<int>("textX", 0);
        textY = Config::get<int>("textY", 0);

        SplashTextAlignment splashTextAlign = getTextAlignment();
        textAlign = splashTextAlign.textAlignment;
        paraAlign = splashTextAlign.paraAlignment;

        wstring splash = Config::get<wstring>("splash", FALLBACK_SPLASH);
        wstring spinner = Config::get<wstring>("spinner", FALLBACK_SPINNER);

        wchar_t dllPath[MAX_PATH];
        GetModuleFileName(hModule, dllPath, MAX_PATH);
        filesystem::path splashPath =
            filesystem::path(dllPath).parent_path() / L"Data\\SKSE\\Plugins\\SplashScreenNG\\";
        splashPath /= splash;
        splashFile = splashPath.wstring();

        pSpinnerFrames = new vector<ComPtr<ID2D1Bitmap>>();

        filesystem::path spinnerPath =
            filesystem::path(dllPath).parent_path() / L"Data\\SKSE\\Plugins\\SplashScreenNG\\";
        spinnerPath /= spinner;
        spinnerFile = spinnerPath.wstring();
    }

    void Splash::CreateSplashWindow() {
        string windowStyleName = Config::get<string>("windowStyle", "forced");
        uint32_t windowStyle = Config::getFrom<int>("windowStyles", windowStyleName, WS_EX_TOPMOST | WS_EX_NOACTIVATE);
        windowStyle |= WS_EX_LAYERED;  // force layered

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int splashPositionX = (screenWidth / 2) - (width / 2);
        int splashPositionY = (screenHeight / 2) - (height / 2);

        hModule = GetModuleHandle(nullptr);

        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hModule;
        wc.lpszClassName = CLASS_NAME;

        if (!RegisterClass(&wc)) log::info("Failed to register window class!");

        log::info("Window Style: {} = {:#010x}", windowStyleName, windowStyle);
        log::info("Splash Size: {} x {}", width, height);

        log::info(
            "Splash File: {}", 
            std::string(splashFile.begin(), splashFile.end())
        );
        
        log::info(
            "Spinner File: {}", 
            std::string(spinnerFile.begin(), spinnerFile.end())
        );

        hSplash = CreateWindowEx(windowStyle, CLASS_NAME, L"", WS_POPUP, splashPositionX, splashPositionY, width,
                                 height, nullptr, nullptr, hModule, nullptr);

        HRGN hitRegion = CreateRectRgn(0, 0, width, height);

        SetWindowRgn(hSplash, hitRegion, FALSE);

        if (FAILED(InitializeD2D())) {
            log::error("InitializeD2D failed");
            return;
        }
        log::info("Initialized Direct2D");

        if (SUCCEEDED(InitializeSKSELog())) {
            log::info("Initialized SKSE64 Log");
            log::info("Path: {}", Splash::SKSELogPath);
        }

        ShowWindow(hSplash, SW_SHOWNOACTIVATE);

        sRunning = true;
    }

    void Splash::ThreadEntry() {
        sThreadId = std::format("{}", std::this_thread::get_id());
        log::info("Thread ID: {}", sThreadId);

        SetupConfig();
        CreateSplashWindow();
     
        MSG msg = {};
        if (!fadeIn) currentOpacity = targetOpacity;
        while (sRunning) {
            UpdateSKSELogLine();

            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            if (currentOpacity < targetOpacity && !_isClosing) currentOpacity += fadeStep;
            if (currentOpacity > 0.0 && _isClosing) {
                currentOpacity -= fadeStep;
                if (currentOpacity <= 0.0f) {
                    fadeOut = false;
                    CloseSplash();
                }
            }

            if (pRenderTarget) RenderFrame();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            framesElapsed += 1;
        }
    }

    void Splash::ShowSplash() {
        if (sRunning) return;
        log::info("Creating Splash Thread");
        sThread = std::thread(ThreadEntry);
        sThread.detach();
    }

    void Splash::CloseSplash() { PostMessage(hSplash, WM_SPLASH_CLOSE, 0, 0); }

    HRESULT Splash::InitializeSKSELog() {
        auto pathOpt = log::log_directory();
        if (!pathOpt) return E_FAIL;

        filesystem::path path = *pathOpt / "skse64.log";
        wstring pathStr = path.wstring();

        DWORD attrib = GetFileAttributes(pathStr.c_str());
        if (attrib == INVALID_FILE_ATTRIBUTES || (attrib & FILE_ATTRIBUTE_DIRECTORY)) return E_FAIL;

        SKSELogPath = path.string();
        sSKSELogFile.open(SKSELogPath);
        return S_OK;
    }

    void Splash::UpdateSKSELogLine() {
        if (!sSKSELogFile.is_open()) return;
        string line;
        if (std::getline(sSKSELogFile, line)) {
            if (line != SKSELastLogLine) SKSELastLogLine = line;
        }
        sSKSELogFile.clear();
    }
}