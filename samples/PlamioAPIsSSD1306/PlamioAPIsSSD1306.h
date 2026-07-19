#include "PLAMIO.h"

#pragma once

// -----------------------------------------------------------------------------
// PLAMIO APIs - SSD1306 Test
//
// Minimal hardware test for SSD1306 systems.
// Tests Graphics continuously, Input on the same screen, and Audio with SE.
// Storage and independent Audio test screens are intentionally omitted.
// -----------------------------------------------------------------------------

class PlamioAPIsSSD1306 : public PLAMIO::Game {
public:
    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

    void onInit(PLAMIO::Storage& storage) override;
    GameState onUpdate(PLAMIO::Input& input,
                       PLAMIO::Audio& audio,
                       PLAMIO::Storage& storage,
                       float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    enum class Mode : uint8_t {
        TITLE,
        GRAPHICS
    };

    void enterGraphics(PLAMIO::Audio& audio);
    void changeStep(int8_t amount, PLAMIO::Audio& audio);
    void playTestSE(PLAMIO::Audio& audio,
                    const PLAMIO::Audio::Sound* sound,
                    float gain,
                    uint16_t iconMsec);

    void updateInputMask(PLAMIO::Input& input);
    void updateTitle(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateGraphics(PLAMIO::Input& input, PLAMIO::Audio& audio);

    void drawTitle(PLAMIO::Graphics& graphics);
    void drawGraphicsTest(PLAMIO::Graphics& graphics);
    void drawMovingShape(PLAMIO::Graphics& graphics,
                         uint8_t step,
                         int16_t x,
                         int16_t y);
    void drawFontTest(PLAMIO::Graphics& graphics);
    void drawAlignmentTest(PLAMIO::Graphics& graphics);
    void drawViewportTest(PLAMIO::Graphics& graphics);
    void drawSpriteTest(PLAMIO::Graphics& graphics);
    void drawInputOverlay(PLAMIO::Graphics& graphics);
    void drawSpeakerIcon(PLAMIO::Graphics& graphics, int16_t x, int16_t y);

    int16_t getAnimX(uint32_t now) const;
    const char* getStepName() const;

    Mode mode = Mode::TITLE;
    uint8_t drawStep = 0;
    uint16_t inputMask = 0;
    uint32_t stepStartMsec = 0;
    uint32_t speakerIconUntilMsec = 0;
};
