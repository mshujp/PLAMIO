#pragma once

#include "PRUZEA.h"

namespace PRUZEA {

class GraphicsBase : public Graphics
{
private:
    int16_t offsetX = 0;
    int16_t offsetY = 0;
    float zoom = 1.0f;
    int16_t zoomCenterX = 0;
    int16_t zoomCenterY = 0;
    bool cameraSuspended = false;

protected:
    int16_t viewportX = 0;
    int16_t viewportY = 0;
    uint16_t logicalScreenW = 0;
    uint16_t logicalScreenH = 0;
    bool screenDirty = false;

    int16_t toScreenX(int16_t x) const;
    int16_t toScreenY(int16_t y) const; 
    uint16_t toScreenW(uint16_t w) const; 
    uint16_t toScreenH(uint16_t h) const; 
    float toScreenScale(float scale) const;

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
    int16_t getViewportX() const;
    int16_t getViewportY() const;
    void resetViewport() override;
    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t thickness, Graphics::Color color) override;
    void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t thickness, Graphics::Color color) override;
    void drawString(const char* str, int16_t x, int16_t y, Color color, Font font, HorizontalAlign ha, VerticalAlign va) override;
    void setCamera(const Camera& camera) override;
    void resetCamera() override;
    void suspendCamera();
    void resumeCamera();
};

} // namespace
