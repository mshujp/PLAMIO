#pragma once
#include "PLAMIO.h"

// -----------------------------------------------------------------------------
// CollisionLab
//
// PLAMIO Sample
//
// Demonstrates:
//
//   * Point vs Rectangle
//   * Rectangle vs Rectangle
//   * Circle vs Rectangle
//   * Circle vs Circle
//
// Learn the Collision API by playing a small game.
// -----------------------------------------------------------------------------

class CollisionLab : public PLAMIO::Game
{
public:
    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

protected:
    void onInit(PLAMIO::Storage& storage) override;
    Game::GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio,
                             PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    enum Mode : uint8_t
    {
        MODE_TITLE,
        MODE_PLAYING,
        MODE_COMPLETE
    };

    enum TestBit : uint8_t
    {
        TEST_POINT_RECT    = 1 << 0,
        TEST_RECT_RECT     = 1 << 1,
        TEST_CIRCLE_CIRCLE = 1 << 2,
        TEST_CIRCLE_RECT   = 1 << 3,
        TEST_ALL           = TEST_POINT_RECT | TEST_RECT_RECT |
                             TEST_CIRCLE_CIRCLE | TEST_CIRCLE_RECT
    };

    static constexpr float PLAYER_RADIUS = 8.0f;
    static constexpr float PLAYER_SPEED = 105.0f;
    static constexpr uint32_t MESSAGE_MSEC = 900;
    static constexpr uint32_t SHAKE_MSEC = 220;
    static constexpr uint32_t COMPLETE_WAIT_MSEC = 1800;

    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;

    Mode mode = MODE_TITLE;

    float playerX = 38.0f;
    float playerY = 120.0f;
    float pointX = 46.0f;
    float pointY = 120.0f;

    uint8_t completedTests = 0;
    uint32_t runStartMsec = 0;
    uint32_t completedMsec = 0;
    uint32_t messageStartMsec = 0;
    uint32_t shakeStartMsec = 0;

    bool messageActive = false;
    bool shakeActive = false;
    char message[32] = {};

    void resetRun();
    void beginTest(uint8_t bit, const char* text, PLAMIO::Audio& audio);
    void updateCollisionTests(PLAMIO::Audio& audio);
    void startShake();
    void drawBackground(PLAMIO::Graphics& g);
    void drawStations(PLAMIO::Graphics& g);
    void drawHud(PLAMIO::Graphics& g);
    void drawTitle(PLAMIO::Graphics& g);
    void drawComplete(PLAMIO::Graphics& g);
};
