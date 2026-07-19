#pragma once

#include "PLAMIO.h"

namespace PLAMIO {

class GraphicsBase : public Graphics
{
protected:
    int16_t viewportX = 0;
    int16_t viewportY = 0;
    uint16_t logicalScreenW = 0;
    uint16_t logicalScreenH = 0;
    bool screenDirty = false;

public:
    enum class CatalogFilterMode
    {
        ExactMatch,
        FitInside,
    };

    virtual ~GraphicsBase() = default;
    virtual const char* getName() const = 0; 
    virtual CatalogFilterMode getCatalogFilterMode() const = 0;
    virtual bool begin() = 0; 
    virtual void end() = 0; 

    virtual uint16_t getScreenWidth() const = 0;
    virtual uint16_t getScreenHeight() const = 0;
    virtual uint16_t getLogicalScreenWidth() const = 0;
    virtual uint16_t getLogicalScreenHeight() const = 0;
    virtual void push() = 0; 

    // 現在ディスプレイに表示される画面の1行をRGB565で取得する。
    // yは実画面上の座標。未対応のGraphicsではfalseを返す。
    virtual bool readScreenLine(uint16_t y, uint16_t* outPixels, uint16_t pixelCount)
    {
        (void)y;
        (void)outPixels;
        (void)pixelCount;
        return false;
    }

    using Graphics::drawString;
    using Graphics::drawRect;
    using Graphics::drawRoundRect;
    uint16_t getTextHeight(const char* text, Font font) override;
    virtual bool setLogicalScreenSize(uint16_t logicalScreenW, uint16_t logicalScreenH);
    void setViewport(int16_t x, int16_t y) override;
    int16_t getViewportX() const { return viewportX; }
    int16_t getViewportY() const { return viewportY; }
    void resetViewport() override;
    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t thickness, Graphics::Color color) override;
    void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t thickness, Graphics::Color color) override;
    void drawString(const char* str, int16_t x, int16_t y, Color color, Font font, HorizontalAlign ha, VerticalAlign va) override;
};

} // namespace
