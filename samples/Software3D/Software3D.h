// -----------------------------------------------------------------------------
// Software3D
//
// PLAMIO Graphics API Sample
//
// Demonstrates:
//
//   * 3D vertex transformation
//   * Perspective projection
//   * Back-face culling
//   * Triangle rasterization
//
// Learn how to build a simple software 3D renderer using
// the PLAMIO Graphics API.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"
#include <cmath>

class Software3D : public PLAMIO::Game {
private:
    // Internal game states
    enum InternalMode {
        MODE_TITLE,
        MODE_PLAYING
    };
    InternalMode currentMode = MODE_TITLE;

    // 3D Vector structure
    struct Vector3D {
        float x, y, z;
    };

    // 2D Coordinates for screen projection
    struct Point2D {
        int16_t x, y;
    };

    // 3D Model definitions (Cube)
    static const int VERTEX_COUNT = 8;
    static const int FACE_COUNT = 6;

    // Local vertex coordinates of the cube
    Vector3D localVertices[VERTEX_COUNT];
    // Vertex indices for each face (defined counter-clockwise for normal calculation)
    uint8_t faceIndices[FACE_COUNT][4];
    // Colors assigned to each face
    PLAMIO::Graphics::Color faceColors[FACE_COUNT];

    // Rotation angles in radians
    float angleX = 0.0f;
    float angleY = 0.0f;
    bool autoRotate = false;

    // Timer and visibility flag for blinking title text
    uint32_t lastBlinkTime = 0;
    bool showPressStart = true;

    // Internal reset function
    void resetGame();

protected:
    // PLAMIO Lifecycle methods
    void onInit(PLAMIO::Storage& storage) override;
    PLAMIO::Game::GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

public:
    // System Metadata
    const char* getId() const override { return "software3D_demo"; }
    const char* getName() const override { return "Software 3D"; }
    const char* getMenuName() const override { return "10 Software 3D"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return PLAMIO::Display::ILI9341_SCREEN_W; }
    uint16_t getLogicalScreenHeight() const override { return PLAMIO::Display::ILI9341_SCREEN_H; }
    uint16_t getTargetScreenWidth() const override { return PLAMIO::Display::ILI9341_SCREEN_W; }
    uint16_t getTargetScreenHeight() const override { return PLAMIO::Display::ILI9341_SCREEN_H; }
};
