// -----------------------------------------------------------------------------
// WaterRipple
//
// PLAMIO Graphics API Sample
//
// Demonstrates:
//
//   * Expanding circular waves
//   * Time-based animation
//   * Damping and lifetime
//   * Layered visual effects
//
// Learn how to create a simple water ripple effect using
// the PLAMIO Graphics API.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class WaterRipple : public PLAMIO::Game
{
public:
    const char* getId() const override { return "waterripple"; }
    const char* getName() const override { return "Water Ripple"; }
    const char* getMenuName() const override { return "06 Water Ripple"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return 320; }
    uint16_t getLogicalScreenHeight() const override { return 240; }
    uint16_t getTargetScreenWidth() const override { return 320; }
    uint16_t getTargetScreenHeight() const override { return 240; }

protected:
    void onInit(PLAMIO::Storage& storage) override;
    GameState onUpdate(PLAMIO::Input& input,
                       PLAMIO::Audio& audio,
                       PLAMIO::Storage& storage,
                       float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    struct Ripple
    {
        float x;
        float y;
        float radius;
        float speed;
        float life;
        float maxLife;
        bool active;
    };

    static constexpr uint8_t MAX_RIPPLES = 12;

    Ripple ripples[MAX_RIPPLES] = {};
    bool autoEmit = false;
    uint32_t lastAutoEmitMsec = 0;
    uint32_t randomState = 0x4D595DF4u;

    void clearRipples();
    void spawnRipple(float x, float y, float speed, float lifetime);
    void updateRipples(float deltaSec);
    uint32_t nextRandom();
    float randomRange(float minimum, float maximum);
};
