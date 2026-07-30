#include "GraphicsILI9341.h"
#include "lgfx/LGFXContext.h"
#include "lgfx/LGFXContextParallel.h"
#include "lgfx/LGFXContextSPI.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <lgfx/utility/lgfx_pngle.h>
#include <new>

#include "pico/stdlib.h"

using namespace PRUZEA;

namespace
{

struct ImageLayout
{
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
};

ImageLayout calculateImageLayout(uint32_t sourceWidth, uint32_t sourceHeight, uint16_t outputWidth, uint16_t outputHeight, Graphics::ImageFit fit)
{
    ImageLayout layout;
    layout.scaleX = static_cast<float>(outputWidth) / sourceWidth;
    layout.scaleY = static_cast<float>(outputHeight) / sourceHeight;

    if (fit != Graphics::ImageFit::STRETCH)
    {
        const float uniformScale = fit == Graphics::ImageFit::CONTAIN
            ? std::min(layout.scaleX, layout.scaleY)
            : std::max(layout.scaleX, layout.scaleY);
        layout.scaleX = uniformScale;
        layout.scaleY = uniformScale;
    }

    const int32_t scaledWidth = static_cast<int32_t>(std::ceil(sourceWidth * layout.scaleX));
    const int32_t scaledHeight = static_cast<int32_t>(std::ceil(sourceHeight * layout.scaleY));
    layout.offsetX = (static_cast<int32_t>(outputWidth) - scaledWidth) / 2;
    layout.offsetY = (static_cast<int32_t>(outputHeight) - scaledHeight) / 2;
    return layout;
}

bool isJpegStartOfFrame(uint8_t marker)
{
    return
        (marker >= 0xC0 && marker <= 0xC3) ||
        (marker >= 0xC5 && marker <= 0xC7) ||
        (marker >= 0xC9 && marker <= 0xCB) ||
        (marker >= 0xCD && marker <= 0xCF);
}

uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
    return
        static_cast<uint16_t>((red & 0xF8) << 8) |
        static_cast<uint16_t>((green & 0xFC) << 3) |
        static_cast<uint16_t>(blue >> 3);
}

} // namespace

struct GraphicsILI9341Image::PngDecodeState
{
    GraphicsILI9341Image* image = nullptr;
    const uint8_t* data = nullptr;
    uint32_t size = 0;
    uint32_t offset = 0;
    ImageLayout layout;
};

GraphicsILI9341Image::GraphicsILI9341Image(LGFX_Device* parent) : sprite(parent)
{
}

bool GraphicsILI9341Image::create(uint16_t outputWidth, uint16_t outputHeight)
{
    if (outputWidth == 0 || outputHeight == 0) return false;

    sprite.setColorDepth(lgfx::color_depth_t::rgb565_nonswapped);
#if PRUZEA_ENABLE_PSRAM
    sprite.setPsram(true);
#endif
    if (sprite.createSprite(outputWidth, outputHeight) == nullptr) return false;

    width = outputWidth;
    height = outputHeight;
    sprite.fillScreen(Graphics::BLACK);
    return true;
}

bool GraphicsILI9341Image::readJpegSize(const uint8_t* data, uint32_t size, uint16_t& outputWidth, uint16_t& outputHeight)
{
    if (data == nullptr || size < 4 || data[0] != 0xFF || data[1] != 0xD8) return false;

    uint32_t offset = 2;
    while (offset + 1 < size)
    {
        while (offset < size && data[offset] != 0xFF) ++offset;
        while (offset < size && data[offset] == 0xFF) ++offset;
        if (offset >= size) return false;

        const uint8_t marker = data[offset++];
        if (marker == 0xD8 || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) continue;
        if (marker == 0xD9 || marker == 0xDA || offset + 1 >= size) return false;

        const uint16_t segmentLength = static_cast<uint16_t>(data[offset] << 8) | data[offset + 1];
        if (segmentLength < 2 || segmentLength > size - offset) return false;

        if (isJpegStartOfFrame(marker))
        {
            if (segmentLength < 7) return false;
            outputHeight = static_cast<uint16_t>(data[offset + 3] << 8) | data[offset + 4];
            outputWidth = static_cast<uint16_t>(data[offset + 5] << 8) | data[offset + 6];
            return outputWidth > 0 && outputHeight > 0;
        }

        offset += segmentLength;
    }
    return false;
}

bool GraphicsILI9341Image::decodeJpeg(const uint8_t* data, uint32_t size, Graphics::ImageFit fit)
{
    uint16_t sourceWidth = 0;
    uint16_t sourceHeight = 0;
    if (!readJpegSize(data, size, sourceWidth, sourceHeight)) return false;

    const ImageLayout layout = calculateImageLayout(sourceWidth, sourceHeight, width, height, fit);
    const int32_t drawX = std::max<int32_t>(layout.offsetX, 0);
    const int32_t drawY = std::max<int32_t>(layout.offsetY, 0);
    const int32_t cropX = std::max<int32_t>(-layout.offsetX, 0);
    const int32_t cropY = std::max<int32_t>(-layout.offsetY, 0);
    return sprite.drawJpg(
        data,
        size,
        drawX,
        drawY,
        width,
        height,
        cropX,
        cropY,
        layout.scaleX,
        layout.scaleY,
        lgfx::datum_t::top_left);
}

uint32_t GraphicsILI9341Image::readPngData(void* userData, uint8_t* buffer, uint32_t length)
{
    auto* state = static_cast<PngDecodeState*>(userData);
    const uint32_t available = state->offset < state->size ? state->size - state->offset : 0;
    const uint32_t readLength = std::min(length, available);

    if (buffer != nullptr && readLength > 0) std::memcpy(buffer, state->data + state->offset, readLength);
    state->offset += readLength;
    return readLength;
}

void GraphicsILI9341Image::drawPngData(void* userData, uint32_t x, uint32_t y, uint_fast8_t divX, size_t length, const uint8_t* argb)
{
    auto* state = static_cast<PngDecodeState*>(userData);
    if (state == nullptr || state->image == nullptr || argb == nullptr) return;

    const int32_t destinationY0 = state->layout.offsetY + static_cast<int32_t>(std::ceil(y * state->layout.scaleY));
    const int32_t destinationY1 = state->layout.offsetY + static_cast<int32_t>(std::ceil((y + 1) * state->layout.scaleY));
    if (destinationY0 >= destinationY1) return;

    for (size_t i = 0; i < length; ++i)
    {
        const int32_t destinationX0 = state->layout.offsetX + static_cast<int32_t>(std::ceil(x * state->layout.scaleX));
        const int32_t destinationX1 = state->layout.offsetX + static_cast<int32_t>(std::ceil((x + divX) * state->layout.scaleX));
        if (destinationX0 < destinationX1)
        {
            const uint16_t color = rgb565(argb[1], argb[2], argb[3]);
            state->image->sprite.fillRect(
                destinationX0,
                destinationY0,
                destinationX1 - destinationX0,
                destinationY1 - destinationY0,
                color);
        }
        x += divX;
        argb += 4;
    }
}

bool GraphicsILI9341Image::decodePng(const uint8_t* data, uint32_t size, Graphics::ImageFit fit)
{
    PngDecodeState state;
    state.image = this;
    state.data = data;
    state.size = size;

    pngle_t* decoder = lgfx_pngle_new();
    if (decoder == nullptr) return false;

    if (lgfx_pngle_prepare(decoder, readPngData, &state) < 0)
    {
        lgfx_pngle_destroy(decoder);
        return false;
    }

    const uint32_t sourceWidth = lgfx_pngle_get_width(decoder);
    const uint32_t sourceHeight = lgfx_pngle_get_height(decoder);
    if (sourceWidth == 0 || sourceHeight == 0)
    {
        lgfx_pngle_destroy(decoder);
        return false;
    }

    state.layout = calculateImageLayout(sourceWidth, sourceHeight, width, height, fit);
    const bool decoded = lgfx_pngle_decomp(decoder, drawPngData) >= 0;
    lgfx_pngle_destroy(decoder);
    return decoded;
}

void GraphicsILI9341Image::drawTo(LGFX_Sprite& destination, int16_t x, int16_t y) const
{
    sprite.pushSprite(&destination, x, y);
}

GraphicsILI9341::GraphicsILI9341(const GraphicsILI9341SPIConfig& config)
    : lgfxContext(std::make_unique<LGFXContextSPI>(config)),
        driverName("ILI9341 SPI"),
        lcdRotate(config.lcdRotate), backLightPin(config.backlightPin),
        canvas(lgfxContext.get()), MAX_BUF_WIDTH(config.maxBufferWidth), MAX_BUF_HEIGHT(config.maxBufferHeight)
{
    logicalScreenW = 0;
    logicalScreenH = 0;
}
GraphicsILI9341::GraphicsILI9341(const GraphicsILI9341ParallelConfig& config)
    : lgfxContext(std::make_unique<LGFXContextParallel>(config)),
        driverName("ILI9341 Parallel"),
        lcdRotate(config.lcdRotate), backLightPin(config.backlightPin),
        canvas(lgfxContext.get()), MAX_BUF_WIDTH(config.maxBufferWidth), MAX_BUF_HEIGHT(config.maxBufferHeight)
{
    logicalScreenW = 0;
    logicalScreenH = 0;
}
GraphicsILI9341::~GraphicsILI9341() = default;

void GraphicsILI9341::setTransformInfo()
{
    uint16_t screenW = getScreenWidth();
    uint16_t screenH = getScreenHeight();

    scale = 1;
    if (logicalScreenW < screenW)
    {
        if (logicalScreenW *2 <= screenW && logicalScreenH * 2 <= screenH)
        {
            scale = 2;
        }
    }
}

void GraphicsILI9341::initSprite(LGFX_Sprite& sprite, uint16_t w, uint16_t h, bool psram)
{
    sprite.setColorDepth(lgfxContext->getColorDepth());
    if (psram) sprite.setPsram(true);
    sprite.createSprite(w, h);
}

bool GraphicsILI9341::setLogicalScreenSize(uint16_t _logicalScreenW, uint16_t _logicalScreenH)
{
    _logicalScreenW = std::clamp(_logicalScreenW, static_cast<uint16_t>(0), MAX_BUF_WIDTH);
    _logicalScreenH = std::clamp(_logicalScreenH, static_cast<uint16_t>(0), MAX_BUF_HEIGHT);
    if (_logicalScreenW == 0 || _logicalScreenH == 0) return true;

    if (logicalScreenW != _logicalScreenW || logicalScreenH != _logicalScreenH)
    {
        if (logicalScreenW > 0)
        {
            canvas.deleteSprite();
            lgfxContext->clear();
        }
        initSprite(canvas, _logicalScreenW, _logicalScreenH);
        lgfxContext->fillScreen(Graphics::Color::DARKGRAY);
    }
    logicalScreenW = _logicalScreenW;
    logicalScreenH = _logicalScreenH;
    setTransformInfo();

    return canvas.getBuffer() != nullptr;
}

bool GraphicsILI9341::begin()
{
    const bool lcdInitialized = lgfxContext->init();
    if (!lcdInitialized) return false;

    lgfxContext->setRotation(lcdRotate);
    lgfxContext->finalizeTouchInitialization();
    canvas.setSwapBytes(true);

    if (backLightPin == 0)
    {
        gpio_init(backLightPin);
        gpio_set_dir(backLightPin, GPIO_OUT);
        gpio_put(backLightPin, 1);
    }
    return true;
}

void GraphicsILI9341::end()
{
    canvas.deleteSprite();
    lgfxContext->clear();
    if (backLightPin > 0) gpio_put(backLightPin, 0);
    screenDirty = false;
}

void GraphicsILI9341::clearScreen()
{
    canvas.clear();
    screenDirty = true;
}

void GraphicsILI9341::fillScreen(Graphics::Color color)
{
    canvas.fillScreen(color);
    screenDirty = true;
}

void GraphicsILI9341::drawPixel(int16_t x, int16_t y, Graphics::Color color)
{
    canvas.drawPixel(x, y, color);
    screenDirty = true;
}

void GraphicsILI9341::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.drawLine(x1, y1, x2, y2, color);
    screenDirty = true;
}

void GraphicsILI9341::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.drawTriangle(x0, y0, x1, y1, x2, y2, color);
    screenDirty = true;
}

void GraphicsILI9341::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    screenDirty = true;
}

void GraphicsILI9341::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color)
{
    canvas.drawRect(x, y, w, h, color);
    screenDirty = true;
}

void GraphicsILI9341::drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, Graphics::Color color)
{
    canvas.drawRoundRect(x, y, w, h, radius, color);
    screenDirty = true;
}

void GraphicsILI9341::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color)
{
    canvas.fillRect(x, y, w, h, color);
    screenDirty = true;
}

void GraphicsILI9341::fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t r, Graphics::Color color)
{
    canvas.fillRoundRect(x, y, w, h, r, color);
    screenDirty = true;
}

void GraphicsILI9341::drawCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color)
{
    canvas.drawCircle(x, y, r, color);
    screenDirty = true;
}

void GraphicsILI9341::drawCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color)
{
    canvas.drawEllipse(x, y, rx, ry, color);
    screenDirty = true;
}

void GraphicsILI9341::fillCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color)
{
    canvas.fillCircle(x, y, r, color);
    screenDirty = true;
}

void GraphicsILI9341::fillCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color)
{
    canvas.fillEllipse(x, y, rx, ry, color);
    screenDirty = true;
}

void GraphicsILI9341::setFont(const char* str, Font font)
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

void GraphicsILI9341::drawString(const char* str, int16_t x, int16_t y, Graphics::Color color, Font font)
{
    if (str == nullptr) return;
    setFont(str, font);
    canvas.setTextColor(color);
    canvas.drawString(str, x, y);
    screenDirty = true;
}

uint16_t GraphicsILI9341::getTextWidth(const char* text, Font font)
{
    if (text == nullptr) return 0;
    setFont(text, font);
    return canvas.textWidth(text);
}

void GraphicsILI9341::drawSprite(const uint16_t* bitmap, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t spriteScale,  Color transparentColor, bool flipX, bool flipY)
{
    if (bitmap == nullptr || spriteScale == 0) return;

    const uint16_t transparent = static_cast<uint16_t>(transparentColor);

    if (spriteScale == 1 && !flipX && !flipY)
    {
        canvas.pushImage(x, y, w, h, bitmap, transparent);
    }
    else
    {
        const float zoomX = flipX ? -static_cast<float>(spriteScale)
                                  :  static_cast<float>(spriteScale);
        const float zoomY = flipY ? -static_cast<float>(spriteScale)
                                  :  static_cast<float>(spriteScale);

        canvas.pushImageRotateZoom(
            x + static_cast<int16_t>((w * spriteScale) / 2), y + static_cast<int16_t>((h * spriteScale) / 2),
            w / 2, h / 2,
            0.0f,
            zoomX, zoomY,
            w, h,
            bitmap,
            transparent
        );
    }

    screenDirty = true;
}

Graphics::Image* GraphicsILI9341::loadJpeg(const uint8_t* jpegData, uint32_t jpegSize, uint16_t outputWidth, uint16_t outputHeight, ImageFit fit)
{
    if (jpegData == nullptr || jpegSize == 0 || outputWidth == 0 || outputHeight == 0) return nullptr;

    auto* image = new (std::nothrow) GraphicsILI9341Image(lgfxContext.get());
    if (image == nullptr) return nullptr;

    if (!image->create(outputWidth, outputHeight) || !image->decodeJpeg(jpegData, jpegSize, fit))
    {
        image->close();
        return nullptr;
    }
    return image;
}

Graphics::Image* GraphicsILI9341::loadPng(const uint8_t* pngData, uint32_t pngSize, uint16_t outputWidth, uint16_t outputHeight, ImageFit fit)
{
    if (pngData == nullptr || pngSize == 0 || outputWidth == 0 || outputHeight == 0) return nullptr;

    auto* image = new (std::nothrow) GraphicsILI9341Image(lgfxContext.get());
    if (image == nullptr) return nullptr;

    if (!image->create(outputWidth, outputHeight) || !image->decodePng(pngData, pngSize, fit))
    {
        image->close();
        return nullptr;
    }
    return image;
}

void GraphicsILI9341::drawImage(const Image& image, int16_t x, int16_t y)
{
    static_cast<const GraphicsILI9341Image&>(image).drawTo(canvas, x, y);
    screenDirty = true;
}

void GraphicsILI9341::setViewport(int16_t x, int16_t y)
{
    if (viewportX == x && viewportY == y) return;
    viewportX = x;
    viewportY = y;
    screenDirty = true;
}

void GraphicsILI9341::push()
{
    if (!screenDirty) return;

    if (scale == 1 && (getScreenWidth() <= getLogicalScreenWidth() && getScreenHeight() <= getLogicalScreenHeight()))
    {
        canvas.pushSprite(lgfxContext.get(), -viewportX, -viewportY);
    }
    else
    {
        float screenCenterX = (float)getScreenWidth() / 2.0f;
        float screenCenterY = (float)getScreenHeight() / 2.0f;
        float targetX = screenCenterX - ((float)viewportX * (float)scale);
        float targetY = screenCenterY - ((float)viewportY * (float)scale);

        canvas.setPivot(canvas.width() / 2, canvas.height() / 2);
        canvas.pushRotateZoom(targetX, targetY, 0.0f, (float)scale, (float)scale);
    }

    screenDirty = false;
}

bool GraphicsILI9341::readScreenLine(uint16_t y, uint16_t* outPixels, uint16_t pixelCount)
{
    if (outPixels == nullptr) return false;

    const uint16_t screenW = getScreenWidth();
    const uint16_t screenH = getScreenHeight();
    if (y >= screenH || pixelCount < screenW || logicalScreenW == 0 || logicalScreenH == 0) return false;

    if (scale == 1)
    {
        const int32_t sourceY = static_cast<int32_t>(y) + viewportY;
        for (uint16_t x = 0; x < screenW; ++x)
        {
            const int32_t sourceX = static_cast<int32_t>(x) + viewportX;
            if (sourceX < 0 || sourceY < 0 || sourceX >= logicalScreenW || sourceY >= logicalScreenH)
            {
                outPixels[x] = static_cast<uint16_t>(Graphics::BLACK);
            }
            else
            {
                outPixels[x] = static_cast<uint16_t>(canvas.readPixel(sourceX, sourceY));
            }
        }
        return true;
    }

    const float screenCenterX = static_cast<float>(screenW) * 0.5f;
    const float screenCenterY = static_cast<float>(screenH) * 0.5f;
    const float targetX = screenCenterX - static_cast<float>(viewportX * scale);
    const float targetY = screenCenterY - static_cast<float>(viewportY * scale);
    const float pivotX = static_cast<float>(logicalScreenW) * 0.5f;
    const float pivotY = static_cast<float>(logicalScreenH) * 0.5f;
    const int32_t sourceY = static_cast<int32_t>((static_cast<float>(y) - targetY) / scale + pivotY);

    for (uint16_t x = 0; x < screenW; ++x)
    {
        const int32_t sourceX = static_cast<int32_t>((static_cast<float>(x) - targetX) / scale + pivotX);
        if (sourceX < 0 || sourceY < 0 || sourceX >= logicalScreenW || sourceY >= logicalScreenH)
        {
            outPixels[x] = static_cast<uint16_t>(Graphics::BLACK);
        }
        else
        {
            outPixels[x] = static_cast<uint16_t>(canvas.readPixel(sourceX, sourceY));
        }
    }

    return true;
}
