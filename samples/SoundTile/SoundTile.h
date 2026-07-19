// -----------------------------------------------------------------------------
// SoundTiles
//
// PLAMIO Audio (Sound Effect) API Sample
//
// Demonstrates:
//
//   * Defining custom sound effects with Audio::SoundStep
//   * Creating Audio::Sound objects
//   * Playing custom sound effects with Audio::playSE()
//
// Select a tile and press A to play the sound.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class SoundTile : public PLAMIO::Game
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
    GameState onUpdate(
        PLAMIO::Input& input,
        PLAMIO::Audio& audio,
        PLAMIO::Storage& storage,
        float deltaSec
    ) override;
    bool onDraw(
        PLAMIO::Graphics& graphics,
        bool requestFullRedraw
    ) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    static constexpr uint8_t COLS = 5;
    static constexpr uint8_t ROWS = 4;
    static constexpr uint8_t TILE_COUNT = COLS * ROWS;

    uint8_t selectedTile = 0;
    uint8_t lastPlayedTile = 255;
    uint32_t flashUntilMsec = 0;

    void moveSelection(int8_t dx, int8_t dy);
};
