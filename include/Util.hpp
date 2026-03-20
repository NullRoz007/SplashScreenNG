#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <Config.h>

namespace SplashNG {
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
            result.textAlignment =
                static_cast<DWRITE_TEXT_ALIGNMENT>(Config::getFrom<int>("textAlignments", textAlignName, 2));
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
        } catch (exception& e) {
            SKSE::log::error("Invalid value passed to textAlign {}", e.what());
        }

        return result;
    }

    DWRITE_FONT_WEIGHT getTextWeight() {
        DWRITE_FONT_WEIGHT result = DWRITE_FONT_WEIGHT_NORMAL;
        string weightName = Config::get<string>("textWeight", "normal");
        try {
            result = static_cast<DWRITE_FONT_WEIGHT>(Config::getFrom<int>("textWeights", weightName, 400));
        } catch (exception& e) {
            SKSE::log::error("Invalid value passed to textWeight {}", e.what());
        }

        return result;
    }

    DWRITE_FONT_STYLE getTextStyle() {
        DWRITE_FONT_STYLE result = DWRITE_FONT_STYLE_NORMAL;
        string styleName = Config::get<string>("textStyle", "normal");
        try {
            result = static_cast<DWRITE_FONT_STYLE>(Config::getFrom<int>("textStyles", styleName, 0));
        } catch (exception& e) {
            SKSE::log::error("Invalid value passed to textStyle {}", e.what());
        }

        return result;
    }

    bool checkOverlap(D2D1_RECT_F a, D2D1_RECT_F b) {
        return (a.left < b.right) && (a.right > b.left) && (a.top < b.bottom) && (a.bottom > b.top);
    }
}