#include "../include/SplashBitmap.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace SKSE;
using namespace DirectX;

namespace SplashNG {
ComPtr<IWICImagingFactory> SplashBitmap::pIWICFactory = nullptr;

HRESULT SplashBitmap::GetFramesFromFile(
    ID2D1RenderTarget* pRenderTarget, PCWSTR path, UINT destWidth,
    UINT destHeight, std::vector<ComPtr<ID2D1Bitmap>>* ppBitmaps) {
  if (!pIWICFactory) return E_POINTER;
  if (!pRenderTarget || !ppBitmaps) return E_POINTER;

  ComPtr<IWICFormatConverter> pConverter = nullptr;
  ComPtr<IWICBitmapScaler> pScaler = nullptr;
  ComPtr<IWICBitmapDecoder> pDecoder = nullptr;

  HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
      path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
      pDecoder.GetAddressOf());

  if (FAILED(hr)) {
    log::error(
        "pIWICFactory->CreateDecoderFromFilename failed (HRESULT {:#010x})",
        static_cast<unsigned>(hr));
    return hr;
  }

  UINT frameCount = 0;
  pDecoder->GetFrameCount(&frameCount);

  for (UINT i = 0; i < frameCount; i++) {
    ComPtr<IWICBitmapFrameDecode> pFrameSource = nullptr;
    ComPtr<ID2D1Bitmap> pBitmap = nullptr;

    hr = pDecoder->GetFrame(i, pFrameSource.GetAddressOf());
    if (FAILED(hr)) {
      log::error("pDecoder->GetFrame failed (HRESULT {:#010x})",
                 static_cast<unsigned>(hr));
      break;
    }

    hr = pIWICFactory->CreateBitmapScaler(pScaler.GetAddressOf());
    if (FAILED(hr)) {
      log::error("pIWICFactory->CreateBitmapScaler failed (HRESULT {:#010x})",
                 static_cast<unsigned>(hr));
      break;
    }

    hr = pScaler->Initialize(pFrameSource.Get(), destWidth, destHeight,
                             WICBitmapInterpolationModeHighQualityCubic);
    if (FAILED(hr)) {
      log::error("pScaler->Initialize failed (HRESULT {:#010x})",
                 static_cast<unsigned>(hr));
      break;
    }

    hr = pIWICFactory->CreateFormatConverter(pConverter.GetAddressOf());
    if (FAILED(hr)) {
      log::error(
          "pIWICFactory->CreateFormatConverter failed (HRESULT {:#010x})",
          static_cast<unsigned>(hr));
      break;
    }

    hr = pConverter->Initialize(pScaler.Get(), GUID_WICPixelFormat32bppPBGRA,
                                WICBitmapDitherTypeNone, nullptr, 0.f,
                                WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
      log::error("pConverter->Initialize failed (HRESULT {:#010x})",
                 static_cast<unsigned>(hr));
      break;
    }

    hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter.Get(), nullptr,
                                                  pBitmap.GetAddressOf());
    if (FAILED(hr)) {
      log::error(
          "pRenderTarget->CreateBitmapFromWicBitmap failed (HRESULT {:#010x})",
          static_cast<unsigned>(hr));
      break;
    }

    ppBitmaps->push_back(pBitmap.Get());
  }

  return hr;
}

HRESULT SplashBitmap::LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget,
                                         PCWSTR path, UINT destWidth,
                                         UINT destHeight,
                                         ID2D1Bitmap** ppBitmap,
                                         vector<uint8_t>* pPixels) {
  if (!pIWICFactory) return E_POINTER;
  if (!pRenderTarget || !ppBitmap) return E_POINTER;

  ComPtr<IWICBitmapFrameDecode> pSource = nullptr;
  ComPtr<IWICFormatConverter> pConverter = nullptr;
  ComPtr<IWICBitmapScaler> pScaler = nullptr;
  ComPtr<IWICBitmapDecoder> pDecoder = nullptr;
  HRESULT hr = E_FAIL;

  if (filesystem::path(path).extension() == ".dds") {
    TexMetadata meta;
    auto scratchImage = make_unique<ScratchImage>();
    hr = LoadFromDDSFile(path, DDS_FLAGS_NONE, &meta, *scratchImage);

    if (FAILED(hr)) {
      wstring pathS = wstring(path);
      log::error("LoadFromDDSFile failed (HRESULT {:#010x})",
                 static_cast<unsigned>(hr));
      return hr;
    }

    log::info("Converting dds -> png");
    int width = meta.width;
    int height = meta.height;
    int depth = meta.depth;

    log::info("w={}, h={}, d={}", width, height, depth);
    if (IsCompressed(meta.format)) {
      log::info("Decompressing dds");
      auto decompressed = make_unique<ScratchImage>();
      hr = Decompress(scratchImage->GetImages(), scratchImage->GetImageCount(),
                      meta, meta.format, *decompressed);

      scratchImage = std::move(decompressed);
      meta = scratchImage->GetMetadata();
    }

    if (meta.format != DXGI_FORMAT_R8G8B8A8_UNORM) {
      log::info("Converting to DXGI_FORMAT_B8G8R8A8_UNORM");
      auto converted = make_unique<ScratchImage>();
      hr = Convert(scratchImage->GetImages(), scratchImage->GetImageCount(),
                   meta, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_FILTER_DEFAULT,
                   TEX_THRESHOLD_DEFAULT, *converted);

      scratchImage = std::move(converted);
      meta = scratchImage->GetMetadata();
    }

    if (scratchImage->GetImageCount() == 0) return E_FAIL;
    const Image* image = scratchImage->GetImage(0, 0, 0);
    if (!image) return E_FAIL;

    ComPtr<IWICBitmap> wicBitmap;
    hr = pIWICFactory->CreateBitmapFromMemory(
        image->width, image->height, GUID_WICPixelFormat32bppPRGBA,
        image->rowPitch, image->rowPitch * image->height, image->pixels,
        wicBitmap.GetAddressOf());

    log::info("Converting to WICPixelFormat32bppPRGBA");

    ComPtr<IWICFormatConverter> pConverter;
    hr = pIWICFactory->CreateFormatConverter(pConverter.GetAddressOf());
    hr = pConverter->Initialize(wicBitmap.Get(), GUID_WICPixelFormat32bppPRGBA,
                                WICBitmapDitherTypeNone, nullptr, 0.f,
                                WICBitmapPaletteTypeMedianCut);
    hr = pRenderTarget->CreateBitmapFromWicBitmap(wicBitmap.Get(), nullptr,
                                                  ppBitmap);

    return hr;
  }

  hr = pIWICFactory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ,
                                               WICDecodeMetadataCacheOnLoad,
                                               pDecoder.GetAddressOf());

  if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, pSource.GetAddressOf());
  if (SUCCEEDED(hr))
    hr = pIWICFactory->CreateBitmapScaler(pScaler.GetAddressOf());
  if (SUCCEEDED(hr))
    hr = pScaler->Initialize(pSource.Get(), destWidth, destHeight,
                             WICBitmapInterpolationModeCubic);
  if (SUCCEEDED(hr))
    hr = pIWICFactory->CreateFormatConverter(pConverter.GetAddressOf());
  if (SUCCEEDED(hr)) {
    hr = pConverter->Initialize(pScaler.Get(), GUID_WICPixelFormat32bppPBGRA,
                                WICBitmapDitherTypeNone, nullptr, 0.f,
                                WICBitmapPaletteTypeMedianCut);

    pPixels->resize(destWidth * destHeight * 4);
  }
  if (SUCCEEDED(hr))
    hr = pConverter->CopyPixels(nullptr, destWidth * 4,
                                destWidth * destHeight * 4, pPixels->data());
  if (SUCCEEDED(hr))
    hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter.Get(), nullptr,
                                                  ppBitmap);
  if (FAILED(hr)) {
    log::error(
        "pRenderTarget->CreateBitmapFromWicBitmap failed (HRESULT {:#010x})",
        static_cast<unsigned>(hr));
    return hr;
  }

  return hr;
}
}  // namespace SplashNG