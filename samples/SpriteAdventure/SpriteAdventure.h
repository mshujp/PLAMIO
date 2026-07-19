// -----------------------------------------------------------------------------
// SpriteAdventure
//
// PLAMIO Sprite and Camera Sample
//
// Demonstrates:
//
//   * Sprite drawing with transparency
//   * Frame-based sprite animation
//   * World, camera, and screen coordinates
//   * Camera following without a large back buffer
//   * Off-screen culling
//
// Collect every gem to complete the adventure.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class SpriteAdventure : public PLAMIO::Game
{
public:
    const char* getId() const override { return "spriteadventure"; }
    const char* getName() const override { return "Sprite Adventure"; }
    const char* getMenuName() const override { return "07 Sprite Adventure"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return 320; }
    uint16_t getLogicalScreenHeight() const override { return 240; }
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
    enum class Mode : uint8_t
    {
        TITLE,
        PLAYING,
        COMPLETE
    };

    struct Gem
    {
        float x;
        float y;
        bool collected;
    };

    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;
    static constexpr int16_t GROUND_Y = 198;

    static constexpr float WORLD_W = 1120.0f;
    static constexpr float PLAYER_SPEED = 92.0f;
    static constexpr float PLAYER_RADIUS = 7.0f;

    static constexpr int16_t SPRITE_W = 8;
    static constexpr int16_t SPRITE_H = 8;
    static constexpr uint8_t SPRITE_SCALE = 2;
    static constexpr int16_t DRAW_W = SPRITE_W * SPRITE_SCALE;
    static constexpr int16_t DRAW_H = SPRITE_H * SPRITE_SCALE;

    static constexpr uint8_t GEM_COUNT = 6;
    static constexpr uint32_t WALK_FRAME_MSEC = 130;
    static constexpr uint32_t COMPLETE_WAIT_MSEC = 700;

    Mode mode = Mode::TITLE;

    float playerX = 36.0f;
    float playerY = GROUND_Y - DRAW_H;
    float cameraX = 0.0f;

    bool facingLeft = false;
    bool walking = false;
    uint8_t walkFrame = 0;
    uint32_t lastWalkFrameMsec = 0;
    uint32_t completeMsec = 0;

    Gem gems[GEM_COUNT] = {};
    uint8_t collectedCount = 0;

    void resetAdventure();
    void updatePlayer(
        PLAMIO::Input& input,
        PLAMIO::Audio& audio,
        float deltaSec);
    void updateCamera();
    void collectGems(PLAMIO::Audio& audio);

    int16_t worldToScreenX(float worldX) const;
    bool isVisible(float worldX, int16_t width) const;

    void drawTitle(PLAMIO::Graphics& graphics);
    void drawWorld(PLAMIO::Graphics& graphics);
    void drawBackground(PLAMIO::Graphics& graphics);
    void drawGround(PLAMIO::Graphics& graphics);
    void drawDecorations(PLAMIO::Graphics& graphics);
    void drawGems(PLAMIO::Graphics& graphics);
    void drawPlayer(PLAMIO::Graphics& graphics);
    void drawHud(PLAMIO::Graphics& graphics);
};
