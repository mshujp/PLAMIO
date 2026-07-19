#pragma once
#include "PLAMIO.h"

class PLAMIOMusicEditor : public PLAMIO::Game
{
public:
    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;
    const char* getMenuGroup() const override { return "TOOLS"; }
    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

protected:
    void onInit(PLAMIO::Storage& storage) override;
    GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio,
                       PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    TerminateResponse onRequestTerminate() override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    static constexpr uint8_t STEP_COUNT = 16;
    static constexpr uint8_t PITCH_COUNT = 8;
    static constexpr uint8_t SLOT_COUNT = 3;
    static constexpr uint8_t MAX_PLAYBACK_NOTES = 32;
    static constexpr uint16_t BPM_MIN = 60;
    static constexpr uint16_t BPM_MAX = 200;
    static constexpr uint16_t BPM_STEP = 10;
    static constexpr uint16_t BPM_DEFAULT = 120;

    enum class Mode : uint8_t
    {
        EDIT,
        MAIN_MENU,
        SAVE_SLOTS,
        LOAD_SLOTS,
        CONFIRM_OVERWRITE,
        CONFIRM_LOAD,
        CONFIRM_CLEAR,
        CONFIRM_EXIT
    };

    struct GridNote
    {
        int8_t pitch;
        uint8_t lengthSteps;
    };

    struct ReadContext
    {
        PLAMIOMusicEditor* editor;
        bool apply;
        bool valid;
        uint8_t noteIndex;
        uint16_t readBpm;
    };

    GridNote grid[STEP_COUNT];
    GridNote savedGrid[STEP_COUNT]{};
    uint16_t savedBpm = BPM_DEFAULT;
    PLAMIO::Audio::ToneNote playbackNotes[MAX_PLAYBACK_NOTES];
    PLAMIO::Audio::Music playbackMusic{};

    Mode mode = Mode::EDIT;
    uint8_t cursorStep = 0;
    uint8_t cursorPitch = 0;
    uint8_t selectedLength = 1;
    uint16_t bpm = BPM_DEFAULT;
    bool playing = false;
    uint32_t playStartMsec = 0;
    uint8_t playheadStep = 0;

    uint8_t menuIndex = 0;
    uint8_t selectedSlot = 0;
    bool slotExists[SLOT_COUNT]{};
    uint16_t slotBpm[SLOT_COUNT]{};
    uint8_t writeLineIndex = 0;
    uint8_t writeSlot = 0;
    bool terminateAfterSave = false;
    bool terminateRequested = false;

    void loadSample();
    void clearAll();
    void placeNote();
    void eraseAt(uint8_t step);
    void cycleLength();
    void removeOverlaps(uint8_t startStep, uint8_t lengthSteps);
    int8_t findCoveringNoteStart(uint8_t step) const;

    void buildPlayback();
    void startPlayback(PLAMIO::Audio& audio);
    void stopPlayback(PLAMIO::Audio& audio);
    uint32_t stepDurationMsec() const;

    void updateEdit(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateMenu(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage);
    void openMainMenu();
    void captureSavedState();
    bool hasUnsavedChanges() const;
    void moveMenuSelection(int8_t direction, uint8_t itemCount);

    void refreshSlotInfo(PLAMIO::Storage& storage);
    bool saveSlot(PLAMIO::Storage& storage, uint8_t slot);
    bool loadSlot(PLAMIO::Storage& storage, uint8_t slot, bool apply);
    static bool readSlotLine(const char* line, void* arg);
    static bool writeDataLine(std::string& line, void* arg);
    static bool writeSourceLine(std::string& line, void* arg);
    void makeDataFileName(uint8_t slot, char* out, uint8_t outSize) const;
    void makeSourceFileName(uint8_t slot, char* out, uint8_t outSize) const;

    void drawEditor(PLAMIO::Graphics& graphics);
    void drawMainMenu(PLAMIO::Graphics& graphics);
    void drawSlotMenu(PLAMIO::Graphics& graphics, bool saving);
    void drawConfirmation(PLAMIO::Graphics& graphics, const char* title,
                          const char* message, const char* yesText);
    void drawExitConfirmation(PLAMIO::Graphics& graphics);

    static uint16_t pitchFrequency(uint8_t pitch);
    static PLAMIO::Audio::ToneNote::Duration durationForSteps(uint8_t steps);
    static const char* pitchName(uint8_t pitch);
    static const char* lengthName(uint8_t steps);
    static const char* durationSourceName(uint8_t steps);
};
