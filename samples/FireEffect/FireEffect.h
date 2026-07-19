// -----------------------------------------------------------------------------
// FireEffect
//
// PLAMIO Graphics API Sample
//
// Demonstrates:
//
//   * A classic software fire effect
//   * Heat propagation and cooling
//   * Palette-based rendering
//
// This effect is generated without particles.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class FireEffect : public PLAMIO::Game
{
public:
    const char* getId() const override { return "fireeffect"; }
    const char* getName() const override { return "Fire Effect"; }
    const char* getMenuName() const override { return "05 Fire Effect"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return 160; }
    uint16_t getLogicalScreenHeight() const override { return 120; }
    uint16_t getTargetScreenWidth() const override { return 320; }
    uint16_t getTargetScreenHeight() const override { return 240; }

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
    static constexpr int16_t SCREEN_W = 160;
    static constexpr int16_t SCREEN_H = 120;
    static constexpr int16_t FIRE_TOP = 18;
    static constexpr int16_t FIRE_H = SCREEN_H - FIRE_TOP;
    static constexpr uint8_t MAX_HEAT = 63;

    uint8_t heat[FIRE_H][SCREEN_W] = {};
    PLAMIO::Graphics::Color palette[MAX_HEAT + 1] = {};

    bool burning = true;
    uint32_t frameCounter = 0;

    void buildPalette();
    void clearHeat();
    void seedBottom();
    void updateFire();
};
