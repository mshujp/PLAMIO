#pragma once
#include "PRUZEA.h"

class AnalogStick : public PRUZEA::Game
{
public:
    const char* getId() const override { return "analog_stick"; }
    const char* getName() const override { return "Analog Stick"; }
    const char* getMenuName() const override { return "13 Analog Stick"; }
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
    bool onDraw(PRUZEA::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PRUZEA::Storage& storage) override;

private:
    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;
    static constexpr int16_t STICK_Y = 104;
    static constexpr int16_t LEFT_X = 84;
    static constexpr int16_t RIGHT_X = 236;
    static constexpr int16_t STICK_RADIUS = 52;
    static constexpr int16_t MARKER_RANGE = 42;

    bool analogAvailable = false;
    int16_t leftX = 0;
    int16_t leftY = 0;
    int16_t rightX = 0;
    int16_t rightY = 0;
    bool l2Pressed = false;
    bool r2Pressed = false;
    bool l3Pressed = false;
    bool r3Pressed = false;

    static void getMarkerPosition(
        int16_t centerX,
        int16_t centerY,
        int16_t axisX,
        int16_t axisY,
        int16_t& outX,
        int16_t& outY);
    static void formatAxisText(char* buffer, size_t bufferSize, int16_t x, int16_t y);

    void drawStick(
        PRUZEA::Graphics& graphics,
        int16_t centerX,
        const char* label,
        int16_t axisX,
        int16_t axisY,
        bool stickPressed,
        bool triggerPressed,
        const char* stickButtonLabel,
        const char* triggerLabel);
};
