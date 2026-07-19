#pragma once
#include "PLAMIO.h"

class PLAMIOSoundEffectEditor : public PLAMIO::Game
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
    static constexpr uint8_t MAX_STEPS = 8;
    static constexpr uint8_t SLOT_COUNT = 3;
    static constexpr uint16_t FREQ_MIN = 50;
    static constexpr uint16_t FREQ_MAX = 5000;
    static constexpr uint16_t FREQ_STEP = 50;
    static constexpr uint16_t DURATION_MIN = 10;
    static constexpr uint16_t DURATION_MAX = 1000;
    static constexpr uint16_t DURATION_STEP = 10;

    enum class Mode : uint8_t
    {
        EDIT,
        VALUE_EDIT,
        MAIN_MENU,
        SAVE_SLOTS,
        LOAD_SLOTS,
        CONFIRM_OVERWRITE,
        CONFIRM_LOAD,
        CONFIRM_CLEAR,
        CONFIRM_EXIT
    };

    enum class Field : uint8_t
    {
        START_FREQ,
        END_FREQ,
        DURATION,
        START_VOLUME,
        END_VOLUME,
        COUNT
    };

    struct ReadContext
    {
        PLAMIOSoundEffectEditor* editor;
        bool apply;
        bool valid;
        uint8_t stepIndex;
        uint8_t expectedCount;
    };

    PLAMIO::Audio::SoundStep steps[MAX_STEPS]{};
    PLAMIO::Audio::SoundStep savedSteps[MAX_STEPS]{};
    uint8_t savedStepCount = 0;
    PLAMIO::Audio::Sound previewSound{};
    uint8_t stepCount = 0;
    uint8_t selectedStep = 0;
    Field selectedField = Field::START_FREQ;
    Mode mode = Mode::EDIT;

    uint8_t menuIndex = 0;
    uint8_t selectedSlot = 0;
    bool slotExists[SLOT_COUNT]{};
    uint8_t slotStepCount[SLOT_COUNT]{};
    uint8_t writeLineIndex = 0;
    uint8_t writeSlot = 0;
    bool terminateAfterSave = false;
    bool terminateRequested = false;

    void loadPreset();
    void clearAll();
    void addStep();
    void duplicateStep();
    void deleteStep();
    void adjustSelectedValue(int8_t direction, bool coarse);
    void playPreview(PLAMIO::Audio& audio);

    void updateEdit(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateMenu(PLAMIO::Input& input, PLAMIO::Audio& audio,
                    PLAMIO::Storage& storage);
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
    void drawFrequencyGraph(PLAMIO::Graphics& graphics);
    void drawFieldPanel(PLAMIO::Graphics& graphics);

    static const char* fieldName(Field field);
    static float clampVolume(float value);
};
