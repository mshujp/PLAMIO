// -----------------------------------------------------------------------------
// GameTemplate
//
// Copy this template when creating a new PLAMIO game.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class GameTemplate : public PLAMIO::Game {
private:
    // Add your member variables here.

protected:
    void onInit(PLAMIO::Storage& storage) override;
    PLAMIO::Game::GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

public:
    const char* getId() const override { return "game_template"; }
    const char* getName() const override { return "Game Template"; }
    const char* getMenuName() const override { return "Game Template"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return PLAMIO::Display::ILI9341_SCREEN_W; }
    uint16_t getLogicalScreenHeight() const override { return PLAMIO::Display::ILI9341_SCREEN_H; }
    uint16_t getTargetScreenWidth() const override { return PLAMIO::Display::ILI9341_SCREEN_W; }
    uint16_t getTargetScreenHeight() const override { return PLAMIO::Display::ILI9341_SCREEN_H; }
};
