#pragma once
#include "PRUZEA.h"

class PRUZEAMusicEditor : public PRUZEA::Game
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
    void onInit(PRUZEA::Storage& storage) override;
    GameState onUpdate(PRUZEA::Input& input, PRUZEA::Audio& audio,
                       PRUZEA::Storage& storage, float deltaSec) override;
    bool onDraw(PRUZEA::Graphics& graphics, bool requestFullRedraw) override;
    TerminateResponse onRequestTerminate() override;
    void onTerminate(PRUZEA::Storage& storage) override;

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
        PRUZEAMusicEditor* editor;
        bool apply;
        bool valid;
        uint8_t noteIndex;
        uint16_t readBpm;
    };

    GridNote grid[STEP_COUNT];
    GridNote savedGrid[STEP_COUNT]{};
    uint16_t savedBpm = BPM_DEFAULT;
    PRUZEA::Audio::ToneNote playbackNotes[MAX_PLAYBACK_NOTES];
    PRUZEA::Audio::Music playbackMusic{};

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
    void startPlayback(PRUZEA::Audio& audio);
    void stopPlayback(PRUZEA::Audio& audio);
    uint32_t stepDurationMsec() const;

    void updateEdit(PRUZEA::Input& input, PRUZEA::Audio& audio);
    void updateMenu(PRUZEA::Input& input, PRUZEA::Audio& audio, PRUZEA::Storage& storage);
    void openMainMenu();
    void captureSavedState();
    bool hasUnsavedChanges() const;
    void moveMenuSelection(int8_t direction, uint8_t itemCount);

    void refreshSlotInfo(PRUZEA::Storage& storage);
    bool saveSlot(PRUZEA::Storage& storage, uint8_t slot);
    bool loadSlot(PRUZEA::Storage& storage, uint8_t slot, bool apply);
    static bool readSlotLine(const char* line, void* arg);
    static bool writeDataLine(std::string& line, void* arg);
    static bool writeSourceLine(std::string& line, void* arg);
    void makeDataFileName(uint8_t slot, char* out, uint8_t outSize) const;
    void makeSourceFileName(uint8_t slot, char* out, uint8_t outSize) const;

    void drawEditor(PRUZEA::Graphics& graphics);
    void drawMainMenu(PRUZEA::Graphics& graphics);
    void drawSlotMenu(PRUZEA::Graphics& graphics, bool saving);
    void drawConfirmation(PRUZEA::Graphics& graphics, const char* title,
                          const char* message, const char* yesText);
    void drawExitConfirmation(PRUZEA::Graphics& graphics);

    static uint16_t pitchFrequency(uint8_t pitch);
    static PRUZEA::Audio::ToneNote::Duration durationForSteps(uint8_t steps);
    static const char* pitchName(uint8_t pitch);
    static const char* lengthName(uint8_t steps);
    static const char* durationSourceName(uint8_t steps);
};
