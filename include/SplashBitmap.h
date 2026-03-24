#pragma once
#include <wincodec.h>
#include <wincodecsdk.h>
#include <d2d1.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <DirectXTex.h>

#pragma comment(lib, "WindowsCodecs.lib")

using namespace std;
using namespace Microsoft::WRL;
namespace SplashNG {
class SplashBitmap {
 public:
  static ComPtr<IWICImagingFactory> pIWICFactory;
  static HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget,
                                    PCWSTR path, UINT destWidth,
                                    UINT destHeight, ID2D1Bitmap** ppBitmap,
                                    vector<uint8_t>* ppPixels = nullptr);

  static HRESULT GetFramesFromFile(ID2D1RenderTarget* pRenderTarget,
                                   PCWSTR path, UINT destWidth, UINT destHeight,
                                   std::vector<ComPtr<ID2D1Bitmap>>* ppBitmaps);
};
}  // namespace SplashNG