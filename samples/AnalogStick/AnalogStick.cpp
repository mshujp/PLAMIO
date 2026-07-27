#include "AnalogStick.h"
#include <cstdio>

using namespace PRUZEA;

void AnalogStick::onInit(Storage& storage)
{
    (void)storage;

    analogAvailable = false;
    leftX = 0;
    leftY = 0;
    rightX = 0;
    rightY = 0;
    l2Pressed = false;
    r2Pressed = false;
    l3Pressed = false;
    r3Pressed = false;
    dirty = true;
}

Game::GameState AnalogStick::onUpdate(
    Input& input,
    Audio& audio,
    Storage& storage,
    float deltaSec)
{
    (void)storage;
    (void)deltaSec;

    if (input.justPressed(Input::START))
    {
        return GameState::TERMINATE_REQUEST;
    }

    if (input.justPressed(Input::L3) || input.justPressed(Input::R3))
    {
        audio.playSE(&Audio::SE::NO_1, 0.45f);
    }

    const bool newAnalogAvailable = input.hasAnalogSticks();
    const int16_t newLeftX = input.axis(Input::LEFT_X);
    const int16_t newLeftY = input.axis(Input::LEFT_Y);
    const int16_t newRightX = input.axis(Input::RIGHT_X);
    const int16_t newRightY = input.axis(Input::RIGHT_Y);
    const bool newL2Pressed = input.pressed(Input::L2);
    const bool newR2Pressed = input.pressed(Input::R2);
    const bool newL3Pressed = input.pressed(Input::L3);
    const bool newR3Pressed = input.pressed(Input::R3);

    if (analogAvailable != newAnalogAvailable ||
        leftX != newLeftX || leftY != newLeftY ||
        rightX != newRightX || rightY != newRightY ||
        l2Pressed != newL2Pressed || r2Pressed != newR2Pressed ||
        l3Pressed != newL3Pressed || r3Pressed != newR3Pressed)
    {
        analogAvailable = newAnalogAvailable;
        leftX = newLeftX;
        leftY = newLeftY;
        rightX = newRightX;
        rightY = newRightY;
        l2Pressed = newL2Pressed;
        r2Pressed = newR2Pressed;
        l3Pressed = newL3Pressed;
        r3Pressed = newR3Pressed;
        dirty = true;
    }

    return GameState::RUNNING;
}

bool AnalogStick::onDraw(Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty)
    {
        return false;
    }

    graphics.fillScreen(Graphics::BLACK);

    graphics.drawString(
        "ANALOG STICK",
        SCREEN_W / 2,
        5,
        Graphics::WHITE,
        Graphics::SIZE_22B,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawString(
        analogAvailable ? "ANALOG: READY" : "NO ANALOG STICKS",
        SCREEN_W / 2,
        31,
        analogAvailable ? Graphics::GREEN : Graphics::ORANGE,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    drawStick(
        graphics,
        LEFT_X,
        "LEFT",
        leftX,
        leftY,
        l3Pressed,
        l2Pressed,
        "L3",
        "L2");

    drawStick(
        graphics,
        RIGHT_X,
        "RIGHT",
        rightX,
        rightY,
        r3Pressed,
        r2Pressed,
        "R3",
        "R2");

    graphics.drawString(
        "PRESS START TO EXIT",
        SCREEN_W / 2,
        226,
        Graphics::GRAY,
        Graphics::SIZE_10,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    dirty = false;
    return true;
}

void AnalogStick::onTerminate(Storage& storage)
{
    (void)storage;
}

void AnalogStick::getMarkerPosition(
    int16_t centerX,
    int16_t centerY,
    int16_t axisX,
    int16_t axisY,
    int16_t& outX,
    int16_t& outY)
{
    float x = static_cast<float>(axisX);
    float y = static_cast<float>(axisY);
    const float length = Math::length(x, y);

    if (length > 1000.0f)
    {
        const float scale = 1000.0f / length;
        x *= scale;
        y *= scale;
    }

    outX = centerX + static_cast<int16_t>(x * MARKER_RANGE / 1000.0f);
    outY = centerY + static_cast<int16_t>(y * MARKER_RANGE / 1000.0f);
}

void AnalogStick::formatAxisText(
    char* buffer,
    size_t bufferSize,
    int16_t x,
    int16_t y)
{
    std::snprintf(buffer, bufferSize, "X:%5d Y:%5d", x, y);
}

void AnalogStick::drawStick(
    Graphics& graphics,
    int16_t centerX,
    const char* label,
    int16_t axisX,
    int16_t axisY,
    bool stickPressed,
    bool triggerPressed,
    const char* stickButtonLabel,
    const char* triggerLabel)
{
    const Graphics::Color activeColor = stickPressed ? Graphics::YELLOW : Graphics::CYAN;
    const Graphics::Color ringColor = analogAvailable
        ? (stickPressed ? Graphics::YELLOW : Graphics::LIGHTGRAY)
        : Graphics::DARKGRAY;

    graphics.drawString(
        label,
        centerX,
        47,
        Graphics::WHITE,
        Graphics::SIZE_18,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawCircle(centerX, STICK_Y, STICK_RADIUS, ringColor);
    graphics.drawCircle(centerX, STICK_Y, STICK_RADIUS - 1, ringColor);
    graphics.drawLine(
        centerX - STICK_RADIUS + 6,
        STICK_Y,
        centerX + STICK_RADIUS - 6,
        STICK_Y,
        Graphics::DARKGRAY);
    graphics.drawLine(
        centerX,
        STICK_Y - STICK_RADIUS + 6,
        centerX,
        STICK_Y + STICK_RADIUS - 6,
        Graphics::DARKGRAY);
    graphics.drawCircle(centerX, STICK_Y, 4, Graphics::GRAY);

    int16_t markerX = centerX;
    int16_t markerY = STICK_Y;
    getMarkerPosition(centerX, STICK_Y, axisX, axisY, markerX, markerY);

    graphics.fillCircle(
        markerX,
        markerY,
        stickPressed ? 9 : 6,
        analogAvailable ? activeColor : Graphics::DARKGRAY);
    graphics.drawCircle(
        markerX,
        markerY,
        stickPressed ? 9 : 6,
        Graphics::WHITE);

    char axisText[32];
    formatAxisText(axisText, sizeof(axisText), axisX, axisY);
    graphics.drawString(
        axisText,
        centerX,
        163,
        Graphics::WHITE,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    char buttonText[32];
    std::snprintf(
        buttonText,
        sizeof(buttonText),
        "%s:%s  %s:%s",
        stickButtonLabel,
        stickPressed ? "ON" : "OFF",
        triggerLabel,
        triggerPressed ? "ON" : "OFF");

    graphics.drawString(
        buttonText,
        centerX,
        185,
        (stickPressed || triggerPressed) ? Graphics::YELLOW : Graphics::LIGHTGRAY,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    if (stickPressed)
    {
        graphics.drawString(
            "CLICK!",
            centerX,
            205,
            Graphics::YELLOW,
            Graphics::SIZE_13,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::TOP);
    }
}
