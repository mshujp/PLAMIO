#pragma once
#include "PLAMIO.h"

class PLAMIOSpriteEditor : public PLAMIO::Game
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
    static constexpr uint8_t SPRITE_W = 16;
    static constexpr uint8_t SPRITE_H = 16;
    static constexpr uint16_t PIXEL_COUNT = SPRITE_W * SPRITE_H;
    static constexpr uint8_t PALETTE_SIZE = 16;
    static constexpr uint8_t SLOT_COUNT = 3;

    enum class Mode : uint8_t
    {
        EDIT,
        PREVIEW,
        MAIN_MENU,
        SAVE_SLOTS,
        LOAD_SLOTS,
        TRANSFORM_MENU,
        CONFIRM_OVERWRITE,
        CONFIRM_LOAD,
        CONFIRM_CLEAR,
        CONFIRM_EXIT
    };

    struct ReadContext
    {
        PLAMIOSpriteEditor* editor;
        bool apply;
        bool valid;
        uint16_t pixelIndex;
    };

    uint8_t pixels[PIXEL_COUNT]{};
    uint8_t savedPixels[PIXEL_COUNT]{};
    uint16_t renderPixels[PIXEL_COUNT]{};
    uint8_t cursorX = 0;
    uint8_t cursorY = 0;
    uint8_t selectedColor = 1;
    Mode mode = Mode::EDIT;
    uint8_t menuIndex = 0;
    uint8_t selectedSlot = 0;
    bool slotExists[SLOT_COUNT]{};
    uint8_t writeSlot = 0;
    uint16_t writeLineIndex = 0;
    bool terminateAfterSave = false;
    bool terminateRequested = false;

    void loadSample();
    void clearAll();
    void paintPixel();
    void erasePixel();
    void pickColor();
    void cycleColor(int8_t direction);
    void rebuildRenderPixels();
    void flipHorizontal();
    void flipVertical();
    void rotate90();

    void updateEdit(PLAMIO::Input& input, PLAMIO::Audio& audio);
    void updateMenu(PLAMIO::Input& input, PLAMIO::Audio& audio,
                    PLAMIO::Storage& storage);
    void moveMenuSelection(int8_t direction, uint8_t itemCount);
    void openMainMenu();
    void captureSavedState();
    bool hasUnsavedChanges() const;

    void refreshSlotInfo(PLAMIO::Storage& storage);
    bool saveSlot(PLAMIO::Storage& storage, uint8_t slot);
    bool loadSlot(PLAMIO::Storage& storage, uint8_t slot, bool apply);
    static bool readSlotLine(const char* line, void* arg);
    static bool writeDataLine(std::string& line, void* arg);
    static bool writeSourceLine(std::string& line, void* arg);
    void makeDataFileName(uint8_t slot, char* out, uint8_t outSize) const;
    void makeSourceFileName(uint8_t slot, char* out, uint8_t outSize) const;

    void drawEditor(PLAMIO::Graphics& graphics);
    void drawPreview(PLAMIO::Graphics& graphics);
    void drawMainMenu(PLAMIO::Graphics& graphics);
    void drawSlotMenu(PLAMIO::Graphics& graphics, bool saving);
    void drawTransformMenu(PLAMIO::Graphics& graphics);
    void drawConfirmation(PLAMIO::Graphics& graphics, const char* title,
                          const char* message, const char* yesText);
    void drawExitConfirmation(PLAMIO::Graphics& graphics);
    void drawPalette(PLAMIO::Graphics& graphics);
    void drawCanvas(PLAMIO::Graphics& graphics);

    static PLAMIO::Graphics::Color paletteColor(uint8_t index);
};
