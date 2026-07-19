#include "PLAMIO.h"

#pragma once

// -----------------------------------------------------------------------------
// PLAMIO Test
//
// A lightweight game framework with reference samples and tests.
//
// Features:
//   * Graphics
//   * Audio
//   * Input
//   * Storage
//   * Collision
//   * Math
//
// -----------------------------------------------------------------------------

class PlamioAPIs : public PLAMIO::Game {
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
    GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    enum class Mode : uint8_t {
        TITLE,
        DRAW_TEST,
        INPUT_TEST,
        AUDIO_TEST,
        STORAGE_TEST
    };

    enum class AudioRow : uint8_t {
        SE,
        MUSIC
    };

    struct StorageContext {
        PlamioAPIs* self;
        bool wrote;
    };

    static bool writeStorageLine(std::string& line, void* arg);
    static bool readStorageLine(const char* line, void* arg);

    void resetToTitle(PLAMIO::Audio& audio);
    void enterDrawTest(PLAMIO::Audio& audio);
    void enterInputTest(PLAMIO::Audio& audio);
    void enterAudioTest(PLAMIO::Audio& audio);
    void enterStorageTest(PLAMIO::Audio& audio, PLAMIO::Storage& storage);
    void runStorageTest(PLAMIO::Storage& storage);
    void nextDrawStep(PLAMIO::Audio& audio);

    void updateTitle(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage);
    void updateDrawTest(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateInputTest(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateAudioTest(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateStorageTest(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage);

    bool drawTitle(PLAMIO::Graphics& graphics);
    bool drawDrawTest(PLAMIO::Graphics& graphics, bool requestFullRedraw);
    bool drawInputTest(PLAMIO::Graphics& graphics);
    bool drawAudioTest(PLAMIO::Graphics& graphics);
    bool drawStorageTest(PLAMIO::Graphics& graphics);

    void drawBackground(PLAMIO::Graphics& graphics);
    void drawHeader(PLAMIO::Graphics& graphics, const char* label);
    void drawCenteredHint(PLAMIO::Graphics& graphics, const char* text, int16_t y);
    void drawMovingShape(PLAMIO::Graphics& graphics, uint8_t step, int16_t x, int16_t y);
    void drawFontTest(PLAMIO::Graphics& graphics);
    void drawAlignmentTest(PLAMIO::Graphics& graphics);
    void drawViewportTest(PLAMIO::Graphics& graphics);
    void drawSpriteTest(PLAMIO::Graphics& graphics);
    void drawButtonLamp(PLAMIO::Graphics& graphics, int16_t x, int16_t y, int16_t w, int16_t h, const char* label, bool on);

    int16_t getAnimX(uint32_t now) const;
    const char* getDrawStepName() const;
    const char* getSeName() const;
    const PLAMIO::Audio::Music* getSelectedMusic(bool loop) const;

    Mode mode = Mode::TITLE;
    uint8_t titleIndex = 0;
    uint8_t drawStep = 0;
    uint8_t lastDrawStep = 0;
    uint32_t modeStartMsec = 0;
    uint32_t stepStartMsec = 0;
    uint32_t lastVisualMsec = 0;
    uint16_t inputMask = 0;

    AudioRow audioRow = AudioRow::SE;
    uint8_t seIndex = 0;
    uint8_t musicIndex = 0;

    uint32_t storageWriteValue = 0;
    uint32_t storageReadValue = 0;
    bool storageAvailable = false;
    bool storageWriteOk = false;
    bool storageReadOk = false;
    bool storageMatch = false;

    bool saveDataSetOk = false;
    bool saveDataSaveOk = false;
    bool saveDataLoadOk = false;
    bool saveDataMatch = false;
    uint8_t saveDataEntryCount = 0;
    uint16_t saveDataUsedBytes = 0;
};
