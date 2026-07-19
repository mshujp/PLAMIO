// -----------------------------------------------------------------------------
// SL
//
// Inspired by the classic "sl".
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

namespace PLAMIO
{
class SL : public Game {
private:
    uint32_t startTimeMsec = 0;
    float trainX = 0.0f;
    bool isInitialized = false;

    constexpr static int16_t TRAIN_WIDTH = 260; 
    constexpr static float TRAIN_SPEED = 0.18f; 

    struct Smoke {
        float x;
        float y;
        uint8_t stage;        // Index for SMOKE_TEXTS
        uint32_t animTimer;   // Timestamp for the next stage transition
        bool active;
    };
    constexpr static uint8_t MAX_SMOKE = 30;
    Smoke smokeParticles[MAX_SMOKE];
    uint32_t lastSmokeTime = 0;

    void updateSmoke(uint32_t currentMsec);
    void drawSl(Graphics& graphics, int16_t x, int16_t y);

public:
    SL() = default;
    virtual ~SL() override = default;

    virtual const char* getId() const override { return "sl"; }
    virtual const char* getName() const override { return "sl"; }
    virtual const char* getMenuName() const override { return "11 sl"; }
    virtual const char* getMenuGroup() const override { return "SAMPLES"; }

    virtual uint16_t getLogicalScreenWidth() const override { return 320; }
    virtual uint16_t getLogicalScreenHeight() const override { return 240; }
    virtual uint16_t getTargetScreenWidth() const override { return 320; }
    virtual uint16_t getTargetScreenHeight() const override { return 240; }

    void onInit(Storage& storage) override;
    GameState onUpdate(Input& input, Audio& audio, Storage& storage, float lastSec) override;
    bool onDraw(Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(Storage& storage) override;
};

} // namespace PLAMIO
