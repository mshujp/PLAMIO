// -----------------------------------------------------------------------------
// GameTemplate
//
// Copy this template when creating a new PRUZEA game.
// -----------------------------------------------------------------------------

#pragma once
#include "PRUZEA.h"

class GameTemplate : public PRUZEA::Game {
private:
    // Add your member variables here.

protected:
    void onInit(PRUZEA::Storage& storage) override;
    PRUZEA::Game::GameState onUpdate(PRUZEA::Input& input, PRUZEA::Audio& audio, PRUZEA::Storage& storage, float deltaSec) override;
    bool onDraw(PRUZEA::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PRUZEA::Storage& storage) override;

public:
    const char* getId() const override { return "game_template"; }
    const char* getName() const override { return "Game Template"; }
    const char* getMenuName() const override { return "Game Template"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return PRUZEA::Display::ILI9341_SCREEN_W; }
    uint16_t getLogicalScreenHeight() const override { return PRUZEA::Display::ILI9341_SCREEN_H; }
    uint16_t getTargetScreenWidth() const override { return PRUZEA::Display::ILI9341_SCREEN_W; }
    uint16_t getTargetScreenHeight() const override { return PRUZEA::Display::ILI9341_SCREEN_H; }
};
