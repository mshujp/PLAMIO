// -----------------------------------------------------------------------------
// WireframeTunnel
//
// PRUZEA Graphics API Sample
//
// Demonstrates:
//
//   * Perspective projection
//   * Wireframe rendering
//   * Depth-based motion
//   * Reusing objects in a looping 3D space
//
// Fly through a simple 3D tunnel built only with lines.
// -----------------------------------------------------------------------------

#pragma once
#include "PRUZEA.h"

class WireframeTunnel : public PRUZEA::Game
{
public:
    const char* getId() const override { return "wireframetunnel"; }
    const char* getName() const override { return "Wireframe Tunnel"; }
    const char* getMenuName() const override { return "09 Wireframe Tunnel"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return 320; }
    uint16_t getLogicalScreenHeight() const override { return 240; }
    uint16_t getTargetScreenWidth() const override { return 320; }
    uint16_t getTargetScreenHeight() const override { return 240; }

protected:
    void onInit(PRUZEA::Storage& storage) override;
    GameState onUpdate(
        PRUZEA::Input& input,
        PRUZEA::Audio& audio,
        PRUZEA::Storage& storage,
        float deltaSec) override;
    bool onDraw(
        PRUZEA::Graphics& graphics,
        bool requestFullRedraw) override;
    void onTerminate(PRUZEA::Storage& storage) override;

private:
    struct Ring
    {
        float z;
        float twist;
    };

    struct Point2D
    {
        int16_t x;
        int16_t y;
        bool visible;
    };

    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;
    static constexpr int16_t CENTER_X = SCREEN_W / 2;
    static constexpr int16_t CENTER_Y = 118;

    static constexpr uint8_t RING_COUNT = 18;
    static constexpr float RING_SPACING = 22.0f;
    static constexpr float NEAR_Z = 16.0f;
    static constexpr float FAR_Z = NEAR_Z + RING_SPACING * RING_COUNT;
    static constexpr float HALF_W = 62.0f;
    static constexpr float HALF_H = 43.0f;
    static constexpr float FOCAL = 170.0f;
    static constexpr float FORWARD_SPEED = 82.0f;
    static constexpr float VIEW_SPEED = 72.0f;

    Ring rings[RING_COUNT] = {};

    float cameraX = 0.0f;
    float cameraY = 0.0f;
    float manualX = 0.0f;
    float manualY = 0.0f;
    float timeSec = 0.0f;

    bool autoDrift = true;

    void resetTunnel();
    void updateRings(float deltaSec);
    void updateView(PRUZEA::Input& input, float deltaSec);

    Point2D project(float x, float y, float z) const;
    PRUZEA::Graphics::Color depthColor(float z) const;

    void drawTunnel(PRUZEA::Graphics& graphics);
    void drawRing(
        PRUZEA::Graphics& graphics,
        const Ring& ring);
    void drawConnections(
        PRUZEA::Graphics& graphics);
    void drawHud(PRUZEA::Graphics& graphics);
};
