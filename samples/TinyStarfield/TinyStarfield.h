// -----------------------------------------------------------------------------
// TinyStarfield
//
// PLAMIO Graphics API Sample (SSD1306)
//
// Demonstrates:
//
//   * Perspective projection
//   * Starfield animation
//   * Depth-based motion
//   * Reusing objects in a looping 3D space
//
// A tiny starfield optimized for monochrome displays.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class TinyStarfield : public PLAMIO::Game
{
public:
    const char* getId() const override { return "tinystarfield"; }
    const char* getName() const override { return "Tiny Starfield"; }
    const char* getMenuName() const override { return "08 Tiny Starfield"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return PLAMIO::Display::SSD1306_SCREEN_W; }
    uint16_t getLogicalScreenHeight() const override { return PLAMIO::Display::SSD1306_SCREEN_H; }
    uint16_t getTargetScreenWidth() const override { return PLAMIO::Display::ILI9341_SCREEN_W; }
    uint16_t getTargetScreenHeight() const override { return PLAMIO::Display::ILI9341_SCREEN_H; }

protected:
    void onInit(PLAMIO::Storage& storage) override;
    GameState onUpdate(
        PLAMIO::Input& input,
        PLAMIO::Audio& audio,
        PLAMIO::Storage& storage,
        float deltaSec) override;
    bool onDraw(
        PLAMIO::Graphics& graphics,
        bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    struct Star
    {
        float x;
        float y;
        float z;
        float previousZ;
    };

    static constexpr int16_t SCREEN_W = PLAMIO::Display::SSD1306_SCREEN_W;
    static constexpr int16_t SCREEN_H = PLAMIO::Display::SSD1306_SCREEN_H;
    static constexpr int16_t CENTER_X = SCREEN_W / 2;
    static constexpr int16_t CENTER_Y = SCREEN_H / 2;

    static constexpr uint8_t STAR_COUNT = 48;
    static constexpr float FAR_Z = 64.0f;
    static constexpr float NEAR_Z = 1.5f;
    static constexpr float PROJECTION_SCALE = 52.0f;
    static constexpr float NORMAL_SPEED = 18.0f;
    static constexpr float WARP_SPEED = 62.0f;
    static constexpr uint32_t WARP_MSEC = 850;

    Star stars[STAR_COUNT] = {};
    uint32_t warpStartMsec = 0;
    bool warpActive = false;

    void resetStar(Star& star, bool randomDepth);
    void resetAllStars();
    void updateStars(float deltaSec);
    void drawStar(PLAMIO::Graphics& graphics, const Star& star) const;

    bool project(
        float x,
        float y,
        float z,
        int16_t& screenX,
        int16_t& screenY) const;
};
