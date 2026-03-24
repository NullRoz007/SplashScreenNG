#include <wincodec.h>
#include <SplashCursor.h>

using namespace Microsoft::WRL;
using namespace SKSE;

namespace SplashNG {
HRESULT BitmapFromLossless2Tag(DefineBitsLossless2Tag* db,
                               IWICImagingFactory* pIWICImagingFactory,
                               ID2D1DCRenderTarget* rt,
                               ID2D1Bitmap** ppBitmap) {
  ComPtr<IWICBitmap> pIWICBitmap;
  std::vector<uint8_t> pixels(db->bitmapData.size());
  size_t expectedSize = (size_t)db->bitmapWidth * db->bitmapHeight * 4;
  if (db->bitmapData.size() != expectedSize) {
    log::error("bitmapData unexpected size: got {} expected {}",
               db->bitmapData.size(), expectedSize);
    return E_BOUNDS;
  }

  for (int i = 0; i < db->bitmapData.size(); i += 4) {
    uint8_t a = db->bitmapData[i];
    uint8_t r = db->bitmapData[i + 1];
    uint8_t g = db->bitmapData[i + 2];
    uint8_t b = db->bitmapData[i + 3];
    pixels[i] = b;
    pixels[i + 1] = g;
    pixels[i + 2] = r;
    pixels[i + 3] = a;
  }

  HRESULT hr = pIWICImagingFactory->CreateBitmapFromMemory(
      db->bitmapWidth, db->bitmapHeight, GUID_WICPixelFormat32bppPBGRA,
      db->bitmapWidth * 4, pixels.size(), pixels.data(),
      pIWICBitmap.GetAddressOf());

  if (FAILED(hr)) {
    return E_FAIL;
  }

  hr = rt->CreateBitmapFromWicBitmap(pIWICBitmap.Get(), nullptr, ppBitmap);
  if (FAILED(hr)) {
    log::error("CreateBitmapFromWicBitmap failed");
  }

  return hr;
}

HRESULT LoadLosslessBitmaps(std::vector<Tag*> tagList,
                            IWICImagingFactory* pIWICImagingFactory,
                            ID2D1DCRenderTarget* rt,
                            BitmapMap* pLosslessBitmaps) {
  HRESULT hr = S_OK;
  for (Tag* t : tagList) {
    if (DefineBitsLossless2Tag* db = dynamic_cast<DefineBitsLossless2Tag*>(t)) {
      ComPtr<ID2D1Bitmap> pBitmap;
      hr = BitmapFromLossless2Tag(db, pIWICImagingFactory, rt,
                                  pBitmap.GetAddressOf());
      if (SUCCEEDED(hr)) {
        (*pLosslessBitmaps)[db->characterId] = pBitmap;
        log::info("Loaded bitmap: id={}", db->characterId);
      }
    }
  }

  return hr;
}

SplashCursor::SplashCursor(std::string path, ComPtr<ID2D1DCRenderTarget> rt) {
  ComPtr<IWICImagingFactory> pIWICImagingFactory;
  HRESULT hr =
      CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                       IID_PPV_ARGS(pIWICImagingFactory.GetAddressOf()));
  if (FAILED(hr)) {
    log::error("Failed to create pIWICImagingFactory");
  }

  swfParser.readFromFile(path.c_str());

  std::vector<Tag*> tagList = swfParser.getTagList();
  log::info("Read {} tags from cursormenu.swf", tagList.size());

  LoadLosslessBitmaps(swfParser.getTagList(), pIWICImagingFactory.Get(),
                      rt.Get(), &losslessBitmaps);
  int i = 0;
  for (Tag* tag : tagList) {
    if (dynamic_cast<DefineShape3Tag*>(tag)) {
      DefineShape3Tag* dsTag = static_cast<DefineShape3Tag*>(tag);
      BuildSubShapes(dsTag);
    }
  }
  log::info("{} shape(s) loaded", shapeSubShapes.size());
}

void SplashCursor::BuildSubShapes(DefineShape3Tag* shapeTag) {
  log::info("Building subshape for tag: {}", shapeTag->getName());
  std::vector<SubShape> subShapes;
  SubShape current;
  float currentX = 0.0f;
  float currentY = 0.0f;
  int currentFillStyleIndex = 0;
  std::vector<FILLSTYLE>* currentFillStyles =
      &shapeTag->shapes.fillStyles.fillStyles;
  for (SHAPERECORD* sr : shapeTag->shapes.shapeRecords) {
    if (StyleChangeRecord* scRec = dynamic_cast<StyleChangeRecord*>(sr)) {
      if (scRec->stateNewStyles) {
        currentFillStyles = &scRec->fillStyles.fillStyles;
      }

      if (scRec->stateMoveTo) {
        if (!current.records.empty()) {
          subShapes.push_back(current);
          current = SubShape{};
          current.fillStyleIndex = currentFillStyleIndex;
        }

        currentX = scRec->moveDeltaX * TWIPS;
        currentY = scRec->moveDeltaY * TWIPS;
        current.startX = currentX;
        current.startY = currentY;
        current.fillStyles = currentFillStyles;
      }

      if (scRec->stateFillStyle0) {
        currentFillStyleIndex = scRec->fillStyle0;
        current.fillStyleIndex = currentFillStyleIndex;
      } else if (scRec->stateFillStyle1) {
        currentFillStyleIndex = scRec->fillStyle1;
        current.fillStyleIndex = currentFillStyleIndex;
      }
    } else if (!dynamic_cast<EndShapeRecord*>(sr)) {
      current.records.push_back(sr);
    }
  }

  if (!current.records.empty()) subShapes.push_back(current);
  shapeSubShapes[shapeTag] = std::move(subShapes);
}

void SplashCursor::RenderBitmapToTarget(ID2D1Bitmap* bitmap,
                                        ID2D1BitmapRenderTarget* rt) {
  D2D1_RECT_F destRect;
  destRect.left = 0;
  destRect.right = bitmap->GetSize().width;
  destRect.top = 0;
  destRect.bottom = bitmap->GetSize().height;

  rt->DrawBitmap(bitmap, destRect);
}

void SplashCursor::RenderGradientToTarget(FILLSTYLE fillStyle,
                                          SubShape subShape,
                                          ID2D1BitmapRenderTarget* rt,
                                          ID2D1Factory* factory) {
  if (fillStyle.gradient.gradientRecords.empty()) return;
  fillStyle.color = fillStyle.gradient.gradientRecords[0].color;

  RenderSolidToTarget(fillStyle, subShape, rt, factory);
}

void SplashCursor::RenderSolidToTarget(FILLSTYLE fillStyle, SubShape subShape,
                                       ID2D1BitmapRenderTarget* rt,
                                       ID2D1Factory* factory) {
  auto& c = fillStyle.color;
  float a = fillStyle.alpha / 255.0f;
  float r = (c.red / 255.0f);
  float g = (c.green / 255.0f);
  float b = (c.blue / 255.0f);

  log::info("fill color: r={} g={} b={} a={}", r, g, b, a);

  ComPtr<ID2D1SolidColorBrush> brush;
  rt->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &brush);

  ComPtr<ID2D1PathGeometry> geometry;
  factory->CreatePathGeometry(&geometry);
  ComPtr<ID2D1GeometrySink> sink;

  geometry->Open(&sink);

  float currentX = subShape.startX;
  float currentY = subShape.startY;

  sink->BeginFigure(D2D1::Point2F(currentX, currentY),
                    D2D1_FIGURE_BEGIN_FILLED);

  for (SHAPERECORD* sr : subShape.records) {
    if (StraightEdgeRecord* seRec = dynamic_cast<StraightEdgeRecord*>(sr)) {
      currentX += seRec->deltaX * TWIPS;
      currentY += seRec->deltaY * TWIPS;
      sink->AddLine(D2D1::Point2F(currentX, currentY));
    } else if (CurvedEdgeRecord* ceRec = dynamic_cast<CurvedEdgeRecord*>(sr)) {
      float cpX = currentX + ceRec->controlDeltaX * TWIPS;
      float cpY = currentY + ceRec->controlDeltaY * TWIPS;
      float anchorX = cpX + ceRec->anchorDeltaX * TWIPS;
      float anchorY = cpY + ceRec->anchorDeltaY * TWIPS;

      D2D1_BEZIER_SEGMENT bezier;
      bezier.point1 =
          D2D1::Point2F(currentX + (2.0f / 3.0f) * (cpX - currentX),
                        currentY + (2.0f / 3.0f) * (cpY - currentY));
      bezier.point2 = D2D1::Point2F(anchorX + (2.0f / 3.0f) * (cpX - anchorX),
                                    anchorY + (2.0f / 3.0f) * (cpY - anchorY));
      bezier.point3 = D2D1::Point2F(anchorX, anchorY);
      sink->AddBezier(bezier);

      currentX = anchorX;
      currentY = anchorY;
    }
  }

  sink->EndFigure(D2D1_FIGURE_END_CLOSED);
  sink->Close();

  rt->FillGeometry(geometry.Get(), brush.Get());
}

void SplashCursor::CreateCursorBitmap(ComPtr<ID2D1Factory> factory,
                                      ComPtr<ID2D1DCRenderTarget> rt,
                                      float scale) {
  HRESULT hr;
  ComPtr<ID2D1BitmapRenderTarget> bitmapTarget;
  D2D1_SIZE_F size = D2D1::SizeF(32 * scale, 32 * scale);

  D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
      DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

  hr = rt->CreateCompatibleRenderTarget(
      &size, nullptr, &pixelFormat, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
      &bitmapTarget);
  if (FAILED(hr)) {
    log::error("Failed to create cursor render target");
    return;
  }

  D2D1::Matrix3x2F transform =
      D2D1::Matrix3x2F::Scale(scale, scale, D2D1::Point2F(0.0f, 0.0f));
  bitmapTarget->SetTransform(transform);
  bitmapTarget->BeginDraw();

  for (auto& [shapeTag, subShapeList] : shapeSubShapes) {
    for (auto& subShape : subShapeList) {
      if (!subShape.fillStyles) continue;
      auto& fillStyles = *subShape.fillStyles;

      if (subShape.fillStyleIndex <= 0 ||
          subShape.fillStyleIndex > fillStyles.size())
        continue;

      FILLSTYLE& fillStyle = fillStyles[subShape.fillStyleIndex - 1];
      switch (fillStyle.fillStyleType) {
        case FILLSTYLE::REPEATING_BITMAP:
          RenderBitmapToTarget(losslessBitmaps[fillStyle.bitmapId].Get(),
                               bitmapTarget.Get());
          break;
        case FILLSTYLE::SOLID:
          RenderSolidToTarget(fillStyle, subShape, bitmapTarget.Get(),
                              factory.Get());
          break;
        case FILLSTYLE::FOCAL_RADIAL_GRADIENT:  // we're not handling gradients
                                                // yet
        case FILLSTYLE::LINEAR_GRADIENT:
        case FILLSTYLE::RADIAL_GRADIENT:
          RenderGradientToTarget(fillStyle, subShape, bitmapTarget.Get(),
                                 factory.Get());
          break;
        default:
          log::warn(
              "Skipping -> fillStyle.fillStyleType {:d} is not "
              "REPEATING_BITMAP, SOLID or *_GRADIENT",
              fillStyle.fillStyleType);
          continue;
      }
    }
  }

  bitmapTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  bitmapTarget->EndDraw();
  bitmapTarget->GetBitmap(cursorBitmap.GetAddressOf());
}

void SplashCursor::RenderCursor(ComPtr<ID2D1Factory> factory,
                                ComPtr<ID2D1DCRenderTarget> rt, float x,
                                float y) {
  if (!cursorBitmap) {
    log::error("cursorBitmap does not exist");
    return;
  }

  int width = cursorBitmap->GetSize().width;
  int height = cursorBitmap->GetSize().height;
  D2D1_RECT_F destRect = D2D1::RectF(x, y, x + width, y + height);
  rt->DrawBitmap(cursorBitmap.Get(), destRect);
}
}  // namespace SplashNG