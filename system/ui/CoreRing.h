#pragma once

#include "GraphicsBase.h"

namespace PLAMIO
{

class CoreRing
{
public:
    enum Mode : uint8_t
    {
        MODE_IDLE,
        MODE_SPLASH,
        MODE_SYSTEM_UI,
        MODE_INFO,
        MODE_STORAGE_READ,
        MODE_STORAGE_WRITE
    };

public:
    static void setMode(Mode mode);
    static void setPowerWarning(bool warning, bool critical);

    static void notifyStorageRead();
    static void notifyStorageWrite();

    static void update();

    static bool drawOverlay(GraphicsBase& graphics, bool requestRedraw);

    static bool isEffectActive();
    static const char* getPowerStatusText();

    static void drawCoreRing(Graphics& graphics, int cx, int cy, uint8_t radius);

private:
    static void draw(Graphics& graphics, int cx, int cy, uint8_t radius);
    static void drawMini(GraphicsBase& graphics, int cx, int cy);

    static void drawBackgroundRing(Graphics& graphics, int cx, int cy, uint8_t radius);
    static void drawForegroundRing(Graphics& graphics, int cx, int cy, uint8_t radius);
    static void drawSplashRings(Graphics& graphics);
    static void drawSystemUI(Graphics& graphics);
    static void drawInfoRing(Graphics& graphics);

    static Graphics::Color getMainColor();
    static Graphics::Color getSubColor();
};

} // namespace PLAMIO
