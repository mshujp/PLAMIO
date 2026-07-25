#pragma once
#include "PRUZEA.h"

class PRUZEASoundEffectEditor : public PRUZEA::Game
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
        PRUZEASoundEffectEditor* editor;
        bool apply;
        bool valid;
        uint8_t stepIndex;
        uint8_t expectedCount;
    };

    PRUZEA::Audio::SoundStep steps[MAX_STEPS]{};
    PRUZEA::Audio::SoundStep savedSteps[MAX_STEPS]{};
    uint8_t savedStepCount = 0;
    PRUZEA::Audio::Sound previewSound{};
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
    void playPreview(PRUZEA::Audio& audio);

    void updateEdit(PRUZEA::Input& input, PRUZEA::Audio& audio);
    void updateMenu(PRUZEA::Input& input, PRUZEA::Audio& audio,
                    PRUZEA::Storage& storage);
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
    void drawFrequencyGraph(PRUZEA::Graphics& graphics);
    void drawFieldPanel(PRUZEA::Graphics& graphics);

    static const char* fieldName(Field field);
    static float clampVolume(float value);
};
