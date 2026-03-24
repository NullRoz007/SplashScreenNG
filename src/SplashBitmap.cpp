#include "../include/SplashBitmap.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace SKSE;

namespace SplashNG {
    ComPtr<IWICImagingFactory> SplashBitmap::pIWICFactory = nullptr;
    HRESULT SplashBitmap::GetFramesFromFile(
        ID2D1RenderTarget* pRenderTarget, 
        PCWSTR path, 
        UINT destWidth,
        UINT destHeight, 
        std::vector<ComPtr<ID2D1Bitmap>>* ppBitmaps) 
    {
        if (!pIWICFactory) return E_POINTER;
        if (!pRenderTarget || !ppBitmaps) return E_POINTER;

        ComPtr<IWICFormatConverter> pConverter = nullptr;
        ComPtr<IWICBitmapScaler> pScaler = nullptr;
        ComPtr<IWICBitmapDecoder> pDecoder = nullptr;

        HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
            path, 
            nullptr, 
            GENERIC_READ, 
            WICDecodeMetadataCacheOnLoad,
            pDecoder.GetAddressOf()
        );

        if(FAILED(hr)) {
            log::error(
                "pIWICFactory->CreateDecoderFromFilename failed (HRESULT {:#010x})",
                static_cast<unsigned>(hr)
            );

            return hr;
        }

        UINT frameCount = -1;
        pDecoder->GetFrameCount(&frameCount);
        
        for (int i = 0; i < frameCount; i++) {
            ComPtr<IWICBitmapFrameDecode> pFrameSource = nullptr;
            ComPtr<ID2D1Bitmap> pBitmap = nullptr;

            hr = pDecoder->GetFrame(i, pFrameSource.GetAddressOf());
            if (SUCCEEDED(hr)) hr = pIWICFactory->CreateBitmapScaler(pScaler.GetAddressOf());
            else {
                log::error(
                    "pDecoder->GetFrame failed (HRESULT {:#010x})",
                    static_cast<unsigned>(hr)
                );

                return hr;
            }
            
            if (SUCCEEDED(hr)) hr = pScaler->Initialize(pFrameSource.Get(), destWidth, destHeight, WICBitmapInterpolationModeHighQualityCubic);
            else {
                log::error(
                    "pIWICFactory->CreateBitmapScaler(HRESULT {:#010x})",
                    static_cast<unsigned>(hr)
                );

                return hr;
            }

            if (SUCCEEDED(hr)) hr = pIWICFactory->CreateFormatConverter(pConverter.GetAddressOf());
            else {
                log::error(
                    "pScaler->Initialize failed (HRESULT {:#010x})",
                    static_cast<unsigned>(hr)
                );

                return hr;
            }
            if (SUCCEEDED(hr)) {
                hr = pConverter->Initialize(
                    pScaler.Get(), 
                    GUID_WICPixelFormat32bppPBGRA, 
                    WICBitmapDitherTypeNone,
                    nullptr, 
                    0.f, 
                    WICBitmapPaletteTypeMedianCut
                );
            }
            else {
                log::error(
                    "pIWICFactory->CreateFormatConverter failed (HRESULT {:#010x})",
                    static_cast<unsigned>(hr)
                );

                return hr;
            }


            if (SUCCEEDED(hr)) hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter.Get(), nullptr, pBitmap.GetAddressOf());
            else {
                log::error(
                    "pConverter->Initialize failed (HRESULT {:#010x})",
                    static_cast<unsigned>(hr)
                );

                return hr;
            }

            if (SUCCEEDED(hr)) {
                ppBitmaps->push_back(pBitmap.Get());
            }
        }
        
        return hr;
    }

    HRESULT SplashBitmap::LoadBitmapFromFile(
        ID2D1RenderTarget* pRenderTarget, 
        PCWSTR path, 
        UINT destWidth,
        UINT destHeight, 
        ID2D1Bitmap** ppBitmap,
        vector<uint8_t>* pPixels
    ) 
    {
        if (!pIWICFactory) return E_POINTER;
        if (!pRenderTarget || !ppBitmap) return E_POINTER;

        ComPtr<IWICBitmapFrameDecode> pSource = nullptr;
        ComPtr<IWICFormatConverter> pConverter = nullptr;
        ComPtr<IWICBitmapScaler> pScaler = nullptr;
        ComPtr<IWICBitmapDecoder> pDecoder = nullptr;

        HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
            path, nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            pDecoder.GetAddressOf()
        );
        
        if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, pSource.GetAddressOf());
        if (SUCCEEDED(hr)) hr = pIWICFactory->CreateBitmapScaler(pScaler.GetAddressOf());
        if (SUCCEEDED(hr)) hr = pScaler->Initialize(pSource.Get(), destWidth, destHeight, WICBitmapInterpolationModeCubic);
        if (SUCCEEDED(hr)) hr = pIWICFactory->CreateFormatConverter(pConverter.GetAddressOf());
        if (SUCCEEDED(hr)) {
            hr = pConverter->Initialize(
                pScaler.Get(), 
                GUID_WICPixelFormat32bppPBGRA, 
                WICBitmapDitherTypeNone,
                nullptr, 
                0.f, 
                WICBitmapPaletteTypeMedianCut
            );
        }
        
        if (SUCCEEDED(hr)) {
            pPixels->resize(destWidth * destHeight * 4);
            pConverter->CopyPixels(
                nullptr,
                destWidth * 4,
                destWidth * destHeight * 4,
                pPixels->data()
            );

            hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter.Get(), nullptr, ppBitmap);
            


        }

        return hr;
    }
    
}