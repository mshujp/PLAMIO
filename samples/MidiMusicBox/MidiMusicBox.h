// -----------------------------------------------------------------------------
// MIDI Music Box
//
// PRUZEA Embedded MIDI API Sample
//
// Demonstrates:
//   * Playing embedded SMF Format 1 data
//   * Selecting from multiple MIDI songs
//   * Starting, stopping, and switching songs
//
// MIDI data:
//   https://www.ne.jp/asahi/music/myuu/midi/midi.htm
//   MIDI programmer: nagisa matui
// -----------------------------------------------------------------------------

#pragma once
#include "PRUZEA.h"

class MidiMusicBox : public PRUZEA::Game
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
    void onInit(PRUZEA::Storage& storage) override;
    GameState onUpdate(
        PRUZEA::Input& input,
        PRUZEA::Audio& audio,
        PRUZEA::Storage& storage,
        float deltaSec
    ) override;
    bool onDraw(
        PRUZEA::Graphics& graphics,
        bool requestFullRedraw
    ) override;
    void onTerminate(PRUZEA::Storage& storage) override;

private:
    static constexpr uint8_t SONG_COUNT = 3;

    uint8_t selectedSong = 0;
    bool playing = false;
    uint32_t animationStartMsec = 0;

    void selectPrevious(PRUZEA::Audio& audio);
    void selectNext(PRUZEA::Audio& audio);
    void playSelected(PRUZEA::Audio& audio);
    void stop(PRUZEA::Audio& audio);
};
