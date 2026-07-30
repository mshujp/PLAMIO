#pragma once
#include "PRUZEA.h"

class ImageGallery : public PRUZEA::Game
{
public:
    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;
    const char* getMenuGroup() const override;

    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

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
    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;
    static constexpr int16_t JPEG_W = 128;
    static constexpr int16_t JPEG_H = 96;
    static constexpr int16_t PNG_W = 64;
    static constexpr int16_t PNG_H = 64;
    static constexpr float MOVE_SPEED = 110.0f;
    static constexpr float ROTATION_SPEED = 180.0f;

    PRUZEA::Image::ImageData* backgroundImage = nullptr;
    PRUZEA::Image::ImageData* characterImage = nullptr;

    float characterX = 128.0f;
    float characterY = 92.0f;
    float characterAngle = 0.0f;
    bool transparencyEnabled = true;
    bool loadSucceeded = false;

    void closeImages();
    void drawScene(PRUZEA::Graphics& graphics);
    void drawStatus(PRUZEA::Graphics& graphics);
};
