#include "GraphicsSSD1306.h"
#include <algorithm>

using namespace PRUZEA;

GraphicsSSD1306::GraphicsSSD1306(const Config& config)
    : lcd(config.sdaPin, config.sclPin, config.i2cPort,
          config.i2cAddr, config.resetPin),
      canvas(&lcd),
      rotate(config.oledRotate)
{
}

uint16_t GraphicsSSD1306::mono(Graphics::Color color) const
{
    return color == Graphics::SSD1306_OFF || color == Graphics::BLACK ? 0 : 1;
}

bool GraphicsSSD1306::begin()
{
    lcd.init();
    lcd.setRotation(rotate);
    
    canvas.setColorDepth(1);
    canvas.createSprite(Graphics::SSD1306_SCREEN_BUF_W, Graphics::SSD1306_SCREEN_BUF_H);
    canvas.clear();

    logicalScreenW = Graphics::SSD1306_SCREEN_BUF_W;
    logicalScreenH = Graphics::SSD1306_SCREEN_BUF_H;
    viewportX = 0;
    viewportY = 0;

    screenDirty = true;
    return true;
}

void GraphicsSSD1306::end()
{
    canvas.deleteSprite();
    lcd.clear();
    screenDirty = false;
}

bool GraphicsSSD1306::setLogicalScreenSize(uint16_t w, uint16_t h)
{
    if (w == 0 || h == 0) return true;
    logicalScreenW = std::clamp(w, static_cast<uint16_t>(0), Graphics::SSD1306_SCREEN_BUF_W);
    logicalScreenH = std::clamp(h, static_cast<uint16_t>(0), Graphics::SSD1306_SCREEN_BUF_H);
    return canvas.getBuffer() != nullptr;
}

void GraphicsSSD1306::clearScreen()
{
    canvas.clear();
    screenDirty = true;
}

void GraphicsSSD1306::fillScreen(Graphics::Color color)
{
    canvas.fillScreen(mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawPixel(int16_t x, int16_t y, Graphics::Color color)
{
    canvas.drawPixel((int32_t)x, (int32_t)y, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.drawLine(x1, y1, x2, y2, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.drawTriangle(x0, y0, x1, y1, x2, y2, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.fillTriangle(x0, y0, x1, y1, x2, y2, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color)
{
    canvas.drawRect(x, y, w, h, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, Graphics::Color color)
{
    canvas.drawRoundRect(x, y, w, h, radius, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color)
{
    canvas.fillRect(x, y, w, h, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t r, Graphics::Color color)
{
    canvas.fillRoundRect(x, y, w, h, r, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color)
{
    canvas.drawCircle(x, y, r, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::drawCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color)
{
    canvas.drawEllipse(x, y, rx, ry, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::fillCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color)
{
    canvas.fillCircle(x, y, r, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::fillCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color)
{
    canvas.fillEllipse(x, y, rx, ry, mono(color));
    screenDirty = true;
}

void GraphicsSSD1306::setFont(const char* str, Font font)
{
    const lgfx::IFont* targetFont = &fonts::DejaVu12;
    float scaleS = 1.0;

    switch (font)
    {
        case Font::SIZE_10:  targetFont = &fonts::DejaVu9; break;
        case Font::SIZE_13:  targetFont = &fonts::DejaVu12; break;
        case Font::SIZE_18:  targetFont = &fonts::DejaVu18; break;
        case Font::SIZE_22B: targetFont = &fonts::FreeSansBold9pt7b; break;
        case Font::SIZE_25:  targetFont = &fonts::DejaVu24; break;
        case Font::SIZE_25B: targetFont = &fonts::FreeSansBold9pt7b; scaleS = 1.13; break;
        case Font::SIZE_32:  targetFont = &fonts::DejaVu24; scaleS = 1.33; break;
        case Font::SIZE_32B: targetFont = &fonts::FreeSansBold12pt7b; scaleS = 1.10; break;
        case Font::SIZE_42:  targetFont = &fonts::DejaVu40; break;
        case Font::SIZE_42B: targetFont = &fonts::FreeSansBold18pt7b; break;
#if PRUZEA_ENABLE_JAPANESE_FONT
        case Font::SIZE_16J: targetFont = &fonts::efontJA_16; break;
        case Font::SIZE_20J: targetFont = &fonts::efontJA_16; scaleS = 1.25; break;
        case Font::SIZE_32J: targetFont = &fonts::efontJA_16; scaleS = 2; break;
#endif
        default: targetFont = &fonts::DejaVu9; break;
    }
 
    canvas.setFont(targetFont);
    canvas.setTextSize(scaleS);
}

void GraphicsSSD1306::drawString(const char* str, int16_t x, int16_t y, Graphics::Color color, Font font)
{
    if (str == nullptr) return;
    setFont(str, font);
    canvas.setTextColor(mono(color), 0);
    canvas.drawString(str, x, y);
    screenDirty = true;
}

uint16_t GraphicsSSD1306::getTextWidth(const char* text, Font font)
{
    if (text == nullptr) return 0;
    setFont(text, font);
    return canvas.textWidth(text);
}

void GraphicsSSD1306::drawSprite(const uint16_t* bitmap, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
    drawSprite(bitmap, x, y, w, h, SpriteOptions{});
}

void GraphicsSSD1306::drawSprite(const uint16_t* bitmap, int16_t x, int16_t y, uint16_t w, uint16_t h, const SpriteOptions& options)
{
    if (bitmap == nullptr || options.scale == 0) return;

    constexpr auto sourceDepth = lgfx::color_depth_t::rgb565_nonswapped;
    const auto* palette = static_cast<const lgfx::rgb888_t*>(nullptr);
    const bool transformed = options.scale != 1 || options.angle != 0.0f || options.flipX || options.flipY;

    if (!transformed)
    {
        if (options.transparent)
        {
            canvas.pushImage(
                x, y, w, h,
                bitmap,
                static_cast<uint16_t>(options.transparentColor),
                sourceDepth,
                palette);
        }
        else
        {
            canvas.pushImage(x, y, w, h, bitmap, sourceDepth, palette);
        }
    }
    else
    {
        const float zoomX = options.flipX ? -static_cast<float>(options.scale) : static_cast<float>(options.scale);
        const float zoomY = options.flipY ? -static_cast<float>(options.scale) : static_cast<float>(options.scale);
        const float destinationX = x + (w * options.scale) * 0.5f;
        const float destinationY = y + (h * options.scale) * 0.5f;

        if (options.transparent)
        {
            canvas.pushImageRotateZoom(destinationX, destinationY, w * 0.5f, h * 0.5f, Math::radToDeg(options.angle), zoomX, zoomY, w, h, bitmap, static_cast<uint16_t>(options.transparentColor), sourceDepth, palette);
        }
        else
        {
            canvas.pushImageRotateZoom(destinationX, destinationY, w * 0.5f, h * 0.5f, Math::radToDeg(options.angle), zoomX, zoomY, w, h, bitmap, sourceDepth, palette);
        }
    }

    screenDirty = true;
}

bool GraphicsSSD1306::readScreenLine(uint16_t y, uint16_t* outPixels, uint16_t pixelCount)
{
    if (outPixels == nullptr) return false;

    const uint16_t screenW = getScreenWidth();
    const uint16_t screenH = getScreenHeight();

    if (y >= screenH || pixelCount < screenW) return false;
    if (logicalScreenW == 0 || logicalScreenH == 0) return false;

    const int32_t sourceY = static_cast<int32_t>(viewportY) + y;

    for (uint16_t x = 0; x < screenW; ++x)
    {
        const int32_t sourceX = static_cast<int32_t>(viewportX) + x;

        if (sourceX < 0 || sourceY < 0 ||
            sourceX >= logicalScreenW || sourceY >= logicalScreenH)
        {
            outPixels[x] = static_cast<uint16_t>(Graphics::BLACK);
            continue;
        }

        const uint16_t pixel = static_cast<uint16_t>(
            canvas.readPixel(sourceX, sourceY));

        outPixels[x] = pixel == 0
            ? static_cast<uint16_t>(Graphics::BLACK)
            : static_cast<uint16_t>(Graphics::WHITE);
    }

    return true;
}

void GraphicsSSD1306::push()
{
    if (!screenDirty) return;
    canvas.pushSprite(&lcd, -viewportX, -viewportY);
    screenDirty = false;
}
