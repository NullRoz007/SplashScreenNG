#pragma once
#include <d2d1.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <map>

#pragma push_macro("RGB")
#pragma push_macro("RECT")
#undef RGB
#undef RECT
#include <libswf/src/DefineBitsLossless2Tag.h>
#include <libswf/src/DefineShape3Tag.h>
#include <libswf/src/SwfParser.h>
#include <libswf/src/StyleChangeRecord.h>
#include <libswf/src/EndShapeRecord.h>
#include <libswf/src/StraightEdgeRecord.h>
#include <libswf/src/CurvedEdgeRecord.h>
#include <libswf/src/RGBA.h>
#include <libswf/src/FILLSTYLE.h>
#pragma pop_macro("RECT")
#pragma pop_macro("RGB")

using BitmapMap = std::map<uint16_t, Microsoft::WRL::ComPtr<ID2D1Bitmap>>;
namespace SplashNG {
    constexpr float TWIPS = 1.0f / 20.0f;
    struct SubShape {
        std::vector<SHAPERECORD*> records;
        std::vector<FILLSTYLE>* fillStyles = nullptr;
        int fillStyleIndex = 0;
        float startX = 0.0f;
        float startY = 0.0f;
    };

    class SplashCursor {
    public:
        SplashCursor(std::string path, Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> rt);
        void CreateCursorBitmap(
            Microsoft::WRL::ComPtr<ID2D1Factory> factory,
            Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> rt, 
            float scale
        );

        void RenderCursor(
            Microsoft::WRL::ComPtr<ID2D1Factory> factory,
            Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> rt, 
            float x, float y
        );

    private:
        SwfParser swfParser;
        std::map<DefineShape3Tag*, std::vector<SubShape>> shapeSubShapes;
        BitmapMap losslessBitmaps;
        Microsoft::WRL::ComPtr<ID2D1Bitmap> cursorBitmap;
        void RenderBitmapToTarget(ID2D1Bitmap* bitmap, ID2D1BitmapRenderTarget* rt);
        void RenderGradientToTarget(FILLSTYLE fillStyle, SubShape subShape, ID2D1BitmapRenderTarget* rt, ID2D1Factory* factory);
        void RenderSolidToTarget(FILLSTYLE fillStyle, SubShape subShape, ID2D1BitmapRenderTarget* rt, ID2D1Factory* factory);
        void BuildSubShapes(DefineShape3Tag* shapeTag);
    };
}