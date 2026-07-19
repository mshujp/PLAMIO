#include "PLAMIOSoundEffectEditor.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace PLAMIO;

namespace
{
constexpr int16_t GRAPH_X = 39;
constexpr int16_t GRAPH_Y = 46;
constexpr int16_t GRAPH_W = 272;
constexpr int16_t GRAPH_H = 91;
constexpr int16_t PANEL_Y = 145;
constexpr int16_t ROW_H = 15;

constexpr Graphics::Color COLOR_BG = Graphics::rgb565(15, 20, 28);
constexpr Graphics::Color COLOR_PANEL = Graphics::rgb565(25, 33, 44);
constexpr Graphics::Color COLOR_GRID = Graphics::rgb565(60, 75, 92);
constexpr Graphics::Color COLOR_BAR = Graphics::rgb565(105, 125, 145);
constexpr Graphics::Color COLOR_LINE = Graphics::rgb565(65, 205, 190);
constexpr Graphics::Color COLOR_CURSOR = Graphics::rgb565(255, 195, 70);
constexpr Graphics::Color COLOR_POINT = Graphics::rgb565(255, 95, 110);

constexpr const char* MAIN_MENU_ITEMS[] = {
    "RESUME", "SAVE", "LOAD", "CLEAR", "PRESET RESET"
};
constexpr uint8_t MAIN_MENU_COUNT = sizeof(MAIN_MENU_ITEMS) / sizeof(MAIN_MENU_ITEMS[0]);

uint16_t clampU16(int32_t value, uint16_t minValue, uint16_t maxValue)
{
    if (value < static_cast<int32_t>(minValue)) return minValue;
    if (value > static_cast<int32_t>(maxValue)) return maxValue;
    return static_cast<uint16_t>(value);
}
}

const char* PLAMIOSoundEffectEditor::getId() const { return "plamio_seeditor"; }
const char* PLAMIOSoundEffectEditor::getName() const { return "PLAMIO Sound Effect Editor"; }
const char* PLAMIOSoundEffectEditor::getMenuName() const { return "PLAMIO SOUND EFFECT EDITOR"; }
uint16_t PLAMIOSoundEffectEditor::getLogicalScreenWidth() const { return Display::ILI9341_SCREEN_W; }
uint16_t PLAMIOSoundEffectEditor::getLogicalScreenHeight() const { return Display::ILI9341_SCREEN_H; }
uint16_t PLAMIOSoundEffectEditor::getTargetScreenWidth() const { return Display::ILI9341_SCREEN_W; }
uint16_t PLAMIOSoundEffectEditor::getTargetScreenHeight() const { return Display::ILI9341_SCREEN_H; }

void PLAMIOSoundEffectEditor::onInit(Storage& storage)
{
    mode = Mode::EDIT;
    selectedStep = 0;
    selectedField = Field::START_FREQ;
    menuIndex = 0;
    selectedSlot = 0;
    loadPreset();
    refreshSlotInfo(storage);
    captureSavedState();
    terminateAfterSave = false;
    terminateRequested = false;
    dirty = true;
}

Game::GameState PLAMIOSoundEffectEditor::onUpdate(Input& input, Audio& audio,
                                             Storage& storage, float deltaSec)
{
    (void)deltaSec;
    if (mode == Mode::EDIT || mode == Mode::VALUE_EDIT)
        updateEdit(input, audio);
    else
        updateMenu(input, audio, storage);
    return terminateRequested ? GameState::TERMINATE_REQUEST : GameState::RUNNING;
}

void PLAMIOSoundEffectEditor::updateEdit(Input& input, Audio& audio)
{
    if (input.justPressed(Input::SELECT))
    {
        mode = Mode::EDIT;
        openMainMenu();
        audio.playSE(&Audio::SE::NO_1, 0.4f);
        return;
    }

    if (mode == Mode::VALUE_EDIT)
    {
        if (input.repeat(Input::LEFT) || input.repeat(Input::DOWN))
        {
            adjustSelectedValue(-1, input.pressed(Input::Y));
            dirty = true;
        }
        if (input.repeat(Input::RIGHT) || input.repeat(Input::UP))
        {
            adjustSelectedValue(1, input.pressed(Input::Y));
            dirty = true;
        }
        if (input.justPressed(Input::A))
        {
            mode = Mode::EDIT;
            audio.playSE(&Audio::SE::NO_1, 0.35f);
            dirty = true;
        }
        if (input.justPressed(Input::B))
        {
            mode = Mode::EDIT;
            audio.playSE(&Audio::SE::NO_2, 0.35f);
            dirty = true;
        }
        if (input.justPressed(Input::START))
        {
            playPreview(audio);
            dirty = true;
        }
        return;
    }

    if (input.repeat(Input::LEFT) && selectedStep > 0)
    {
        --selectedStep;
        dirty = true;
    }
    if (input.repeat(Input::RIGHT) && selectedStep + 1 < stepCount)
    {
        ++selectedStep;
        dirty = true;
    }
    if (input.repeat(Input::UP))
    {
        const uint8_t value = static_cast<uint8_t>(selectedField);
        selectedField = static_cast<Field>(value == 0 ? static_cast<uint8_t>(Field::COUNT) - 1 : value - 1);
        dirty = true;
    }
    if (input.repeat(Input::DOWN))
    {
        const uint8_t value = static_cast<uint8_t>(selectedField);
        selectedField = static_cast<Field>((value + 1) % static_cast<uint8_t>(Field::COUNT));
        dirty = true;
    }

    if (input.justPressed(Input::A))
    {
        mode = Mode::VALUE_EDIT;
        audio.playSE(&Audio::SE::NO_1, 0.35f);
        dirty = true;
    }
    if (input.justPressed(Input::X))
    {
        addStep();
        audio.playSE(&Audio::SE::NO_3, 0.4f);
        dirty = true;
    }
    if (input.justPressed(Input::Y))
    {
        duplicateStep();
        audio.playSE(&Audio::SE::NO_3, 0.4f);
        dirty = true;
    }
    if (input.justPressed(Input::B))
    {
        deleteStep();
        audio.playSE(&Audio::SE::NO_2, 0.4f);
        dirty = true;
    }
    if (input.justPressed(Input::START))
    {
        playPreview(audio);
        dirty = true;
    }
}

void PLAMIOSoundEffectEditor::updateMenu(Input& input, Audio& audio, Storage& storage)
{
    if (mode == Mode::CONFIRM_EXIT)
    {
        if (input.repeat(Input::UP)) moveMenuSelection(-1, 3);
        if (input.repeat(Input::DOWN)) moveMenuSelection(1, 3);

        if (input.justPressed(Input::B))
        {
            terminateAfterSave = false;
            mode = Mode::EDIT;
            menuIndex = 0;
            audio.playSE(&Audio::SE::NO_2, 0.4f);
            dirty = true;
            return;
        }
        if (!input.justPressed(Input::A)) return;

        audio.playSE(&Audio::SE::NO_1, 0.4f);
        if (menuIndex == 0)
        {
            if (!storage.isAvailable())
            {
                audio.playSE(&Audio::SE::NO_2, 0.4f);
            }
            else
            {
                terminateAfterSave = true;
                mode = Mode::SAVE_SLOTS;
                menuIndex = 0;
            }
        }
        else if (menuIndex == 1)
        {
            terminateRequested = true;
        }
        else
        {
            terminateAfterSave = false;
            mode = Mode::EDIT;
            menuIndex = 0;
        }
        dirty = true;
        return;
    }
    uint8_t itemCount = MAIN_MENU_COUNT;
    if (mode == Mode::SAVE_SLOTS || mode == Mode::LOAD_SLOTS)
        itemCount = SLOT_COUNT;

    if (mode == Mode::MAIN_MENU || mode == Mode::SAVE_SLOTS || mode == Mode::LOAD_SLOTS)
    {
        if (input.repeat(Input::UP)) moveMenuSelection(-1, itemCount);
        if (input.repeat(Input::DOWN)) moveMenuSelection(1, itemCount);

        if (input.justPressed(Input::B))
        {
            if (mode == Mode::SAVE_SLOTS && terminateAfterSave)
                mode = Mode::CONFIRM_EXIT;
            else
                mode = (mode == Mode::MAIN_MENU) ? Mode::EDIT : Mode::MAIN_MENU;
            menuIndex = 0;
            audio.playSE(&Audio::SE::NO_2, 0.4f);
            dirty = true;
            return;
        }

        if (!input.justPressed(Input::A)) return;

        audio.playSE(&Audio::SE::NO_1, 0.4f);
        if (mode == Mode::MAIN_MENU)
        {
            switch (menuIndex)
            {
            case 0: mode = Mode::EDIT; break;
            case 1:
                if (!storage.isAvailable())
                {
                    audio.playSE(&Audio::SE::NO_2, 0.4f);
                    break;
                }
                mode = Mode::SAVE_SLOTS;
                menuIndex = 0;
                break;
            case 2:
                if (!storage.isAvailable())
                {
                    audio.playSE(&Audio::SE::NO_2, 0.4f);
                    break;
                }
                mode = Mode::LOAD_SLOTS;
                menuIndex = 0;
                break;
            case 3: mode = Mode::CONFIRM_CLEAR; break;
            case 4: loadPreset(); mode = Mode::EDIT; break;
            default: break;
            }
        }
        else if (mode == Mode::SAVE_SLOTS)
        {
            selectedSlot = menuIndex;
            if (slotExists[selectedSlot])
                mode = Mode::CONFIRM_OVERWRITE;
            else
            {
                const bool success = saveSlot(storage, selectedSlot);
                refreshSlotInfo(storage);
                audio.playSE(success ? &Audio::SE::NO_3 : &Audio::SE::NO_2, 0.5f);
                if (success && terminateAfterSave)
                    terminateRequested = true;
                else
                    mode = Mode::EDIT;
            }
        }
        else
        {
            selectedSlot = menuIndex;
            if (slotExists[selectedSlot])
                mode = Mode::CONFIRM_LOAD;
            else
                audio.playSE(&Audio::SE::NO_2, 0.5f);
        }
        dirty = true;
        return;
    }

    if (input.justPressed(Input::B))
    {
        if (mode == Mode::CONFIRM_OVERWRITE && terminateAfterSave)
            mode = Mode::SAVE_SLOTS;
        else
            mode = Mode::MAIN_MENU;
        menuIndex = 0;
        audio.playSE(&Audio::SE::NO_2, 0.4f);
        dirty = true;
        return;
    }

    if (!input.justPressed(Input::A)) return;

    bool success = true;
    if (mode == Mode::CONFIRM_OVERWRITE)
    {
        success = saveSlot(storage, selectedSlot);
        refreshSlotInfo(storage);
    }
    else if (mode == Mode::CONFIRM_LOAD)
    {
        success = loadSlot(storage, selectedSlot, true);
    }
    else if (mode == Mode::CONFIRM_CLEAR)
    {
        clearAll();
    }

    audio.playSE(success ? &Audio::SE::NO_3 : &Audio::SE::NO_2, 0.5f);
    if (success && terminateAfterSave && mode == Mode::CONFIRM_OVERWRITE)
        terminateRequested = true;
    else
        mode = Mode::EDIT;
    dirty = true;
}

bool PLAMIOSoundEffectEditor::onDraw(Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty)
        return false;

    drawEditor(graphics);
    switch (mode)
    {
    case Mode::MAIN_MENU: drawMainMenu(graphics); break;
    case Mode::SAVE_SLOTS: drawSlotMenu(graphics, true); break;
    case Mode::LOAD_SLOTS: drawSlotMenu(graphics, false); break;
    case Mode::CONFIRM_OVERWRITE:
        drawConfirmation(graphics, "OVERWRITE?", "Existing sound will be replaced", "A: SAVE");
        break;
    case Mode::CONFIRM_LOAD:
        drawConfirmation(graphics, "LOAD SOUND?", "Current sound will be lost", "A: LOAD");
        break;
    case Mode::CONFIRM_EXIT:
        drawExitConfirmation(graphics);
        break;
    case Mode::CONFIRM_CLEAR:
        drawConfirmation(graphics, "CLEAR ALL?", "All sound steps will be removed", "A: CLEAR");
        break;
    default: break;
    }

    dirty = false;
    return true;
}

void PLAMIOSoundEffectEditor::drawExitConfirmation(Graphics& graphics)
{
    static const char* ITEMS[] = {"SAVE", "DON'T SAVE", "CANCEL"};
    graphics.fillRect(42, 55, 236, 142, COLOR_PANEL);
    graphics.drawRect(42, 55, 236, 142, 2, COLOR_CURSOR);
    graphics.drawString("UNSAVED CHANGES", 160, 66, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < 3; ++i)
    {
        if (i == menuIndex) graphics.fillRoundRect(70, 101 + i * 25, 180, 21, 4, COLOR_LINE);
        graphics.drawString(ITEMS[i], 160, 103 + i * 25,
                            i == menuIndex ? COLOR_BG : Graphics::WHITE, Graphics::SIZE_18,
                            Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    }
}

Game::TerminateResponse PLAMIOSoundEffectEditor::onRequestTerminate()
{
    if (!hasUnsavedChanges()) return TerminateResponse::ACCEPT;
    mode = Mode::CONFIRM_EXIT;
    menuIndex = 0;
    terminateAfterSave = false;
    dirty = true;
    return TerminateResponse::REJECT;
}

void PLAMIOSoundEffectEditor::captureSavedState()
{
    savedStepCount = stepCount;
    for (uint8_t i = 0; i < MAX_STEPS; ++i) savedSteps[i] = steps[i];
}

bool PLAMIOSoundEffectEditor::hasUnsavedChanges() const
{
    if (stepCount != savedStepCount) return true;
    for (uint8_t i = 0; i < stepCount; ++i)
    {
        const Audio::SoundStep& a = steps[i];
        const Audio::SoundStep& b = savedSteps[i];
        if (a.startFrequency != b.startFrequency || a.endFrequency != b.endFrequency ||
            a.durationMsec != b.durationMsec || a.startVolume != b.startVolume ||
            a.endVolume != b.endVolume)
            return true;
    }
    return false;
}

void PLAMIOSoundEffectEditor::onTerminate(Storage& storage)
{
    (void)storage;
}

void PLAMIOSoundEffectEditor::loadPreset()
{
    stepCount = 2;
    steps[0] = {1200, 1800, 50, 1.0f, 0.7f};
    steps[1] = {1800, 2200, 40, 0.7f, 0.0f};
    selectedStep = 0;
    selectedField = Field::START_FREQ;
}

void PLAMIOSoundEffectEditor::clearAll()
{
    stepCount = 1;
    steps[0] = {1000, 1000, 100, 1.0f, 0.0f};
    selectedStep = 0;
    selectedField = Field::START_FREQ;
}

void PLAMIOSoundEffectEditor::addStep()
{
    if (stepCount >= MAX_STEPS) return;
    const uint8_t insertAt = static_cast<uint8_t>(selectedStep + 1);
    for (uint8_t i = stepCount; i > insertAt; --i)
        steps[i] = steps[i - 1];

    const Audio::SoundStep& previous = steps[selectedStep];
    steps[insertAt] = {previous.endFrequency, previous.endFrequency, 100,
                       previous.endVolume, previous.endVolume};
    ++stepCount;
    selectedStep = insertAt;
}

void PLAMIOSoundEffectEditor::duplicateStep()
{
    if (stepCount >= MAX_STEPS) return;
    const uint8_t insertAt = static_cast<uint8_t>(selectedStep + 1);
    for (uint8_t i = stepCount; i > insertAt; --i)
        steps[i] = steps[i - 1];
    steps[insertAt] = steps[selectedStep];
    ++stepCount;
    selectedStep = insertAt;
}

void PLAMIOSoundEffectEditor::deleteStep()
{
    if (stepCount <= 1)
    {
        clearAll();
        return;
    }
    for (uint8_t i = selectedStep; i + 1 < stepCount; ++i)
        steps[i] = steps[i + 1];
    --stepCount;
    if (selectedStep >= stepCount) selectedStep = static_cast<uint8_t>(stepCount - 1);
}

void PLAMIOSoundEffectEditor::adjustSelectedValue(int8_t direction, bool coarse)
{
    Audio::SoundStep& step = steps[selectedStep];
    switch (selectedField)
    {
    case Field::START_FREQ:
    {
        const int32_t amount = coarse ? 500 : FREQ_STEP;
        step.startFrequency = clampU16(static_cast<int32_t>(step.startFrequency) + direction * amount,
                                       FREQ_MIN, FREQ_MAX);
        break;
    }
    case Field::END_FREQ:
    {
        const int32_t amount = coarse ? 500 : FREQ_STEP;
        step.endFrequency = clampU16(static_cast<int32_t>(step.endFrequency) + direction * amount,
                                     FREQ_MIN, FREQ_MAX);
        break;
    }
    case Field::DURATION:
    {
        const int32_t amount = coarse ? 100 : DURATION_STEP;
        step.durationMsec = clampU16(static_cast<int32_t>(step.durationMsec) + direction * amount,
                                     DURATION_MIN, DURATION_MAX);
        break;
    }
    case Field::START_VOLUME:
        step.startVolume = clampVolume(step.startVolume + direction * (coarse ? 0.5f : 0.1f));
        break;
    case Field::END_VOLUME:
        step.endVolume = clampVolume(step.endVolume + direction * (coarse ? 0.5f : 0.1f));
        break;
    default: break;
    }
}

void PLAMIOSoundEffectEditor::playPreview(Audio& audio)
{
    previewSound.steps = steps;
    previewSound.stepCount = stepCount;
    audio.playSE(&previewSound, 1.0f);
}

void PLAMIOSoundEffectEditor::openMainMenu()
{
    mode = Mode::MAIN_MENU;
    menuIndex = 0;
    dirty = true;
}

void PLAMIOSoundEffectEditor::moveMenuSelection(int8_t direction, uint8_t itemCount)
{
    if (direction < 0)
        menuIndex = (menuIndex == 0) ? static_cast<uint8_t>(itemCount - 1) : static_cast<uint8_t>(menuIndex - 1);
    else
        menuIndex = static_cast<uint8_t>((menuIndex + 1) % itemCount);
    dirty = true;
}

void PLAMIOSoundEffectEditor::refreshSlotInfo(Storage& storage)
{
    for (uint8_t i = 0; i < SLOT_COUNT; ++i)
    {
        slotExists[i] = false;
        slotStepCount[i] = 0;
        loadSlot(storage, i, false);
    }
}

bool PLAMIOSoundEffectEditor::saveSlot(Storage& storage, uint8_t slot)
{
    char dataName[16];
    char sourceName[16];
    makeDataFileName(slot, dataName, sizeof(dataName));
    makeSourceFileName(slot, sourceName, sizeof(sourceName));

    writeSlot = slot;
    writeLineIndex = 0;
    const bool dataOk = storage.writeUserFile(getId(), dataName,
                                               &PLAMIOSoundEffectEditor::writeDataLine, this);
    writeLineIndex = 0;
    const bool sourceOk = storage.writeUserFile(getId(), sourceName,
                                                 &PLAMIOSoundEffectEditor::writeSourceLine, this);
    const bool success = dataOk && sourceOk;
    if (success) captureSavedState();
    return success;
}

bool PLAMIOSoundEffectEditor::loadSlot(Storage& storage, uint8_t slot, bool apply)
{
    char fileName[16];
    makeDataFileName(slot, fileName, sizeof(fileName));

    ReadContext context{this, apply, true, 0, 0};
    const bool opened = storage.readUserFile(getId(), fileName,
                                              &PLAMIOSoundEffectEditor::readSlotLine, &context);
    const bool valid = opened && context.valid && context.expectedCount > 0 &&
                       context.stepIndex == context.expectedCount;
    if (valid)
    {
        slotExists[slot] = true;
        slotStepCount[slot] = context.expectedCount;
        if (apply)
        {
            stepCount = context.expectedCount;
            selectedStep = 0;
            selectedField = Field::START_FREQ;
            captureSavedState();
        }
    }
    return valid;
}

bool PLAMIOSoundEffectEditor::readSlotLine(const char* line, void* arg)
{
    ReadContext* context = static_cast<ReadContext*>(arg);
    if (std::strncmp(line, "COUNT=", 6) == 0)
    {
        const int count = std::atoi(line + 6);
        if (count < 1 || count > MAX_STEPS)
        {
            context->valid = false;
            return false;
        }
        context->expectedCount = static_cast<uint8_t>(count);
        return true;
    }

    unsigned startFreq = 0;
    unsigned endFreq = 0;
    unsigned duration = 0;
    unsigned startVolume = 0;
    unsigned endVolume = 0;
    if (std::sscanf(line, "STEP=%u,%u,%u,%u,%u",
                    &startFreq, &endFreq, &duration, &startVolume, &endVolume) == 5)
    {
        if (context->stepIndex >= MAX_STEPS || startFreq < FREQ_MIN || startFreq > FREQ_MAX ||
            endFreq < FREQ_MIN || endFreq > FREQ_MAX || duration < DURATION_MIN ||
            duration > DURATION_MAX || startVolume > 10 || endVolume > 10)
        {
            context->valid = false;
            return false;
        }
        if (context->apply)
        {
            Audio::SoundStep& step = context->editor->steps[context->stepIndex];
            step.startFrequency = static_cast<uint16_t>(startFreq);
            step.endFrequency = static_cast<uint16_t>(endFreq);
            step.durationMsec = static_cast<uint16_t>(duration);
            step.startVolume = static_cast<float>(startVolume) / 10.0f;
            step.endVolume = static_cast<float>(endVolume) / 10.0f;
        }
        ++context->stepIndex;
        return true;
    }

    context->valid = false;
    return false;
}

bool PLAMIOSoundEffectEditor::writeDataLine(std::string& line, void* arg)
{
    PLAMIOSoundEffectEditor* editor = static_cast<PLAMIOSoundEffectEditor*>(arg);
    char buffer[80];

    if (editor->writeLineIndex == 0)
    {
        std::snprintf(buffer, sizeof(buffer), "COUNT=%u", editor->stepCount);
        line.assign(buffer);
        ++editor->writeLineIndex;
        return true;
    }

    const uint8_t index = static_cast<uint8_t>(editor->writeLineIndex - 1);
    if (index < editor->stepCount)
    {
        const Audio::SoundStep& step = editor->steps[index];
        std::snprintf(buffer, sizeof(buffer), "STEP=%u,%u,%u,%u,%u",
                      step.startFrequency, step.endFrequency, step.durationMsec,
                      static_cast<unsigned>(step.startVolume * 10.0f + 0.5f),
                      static_cast<unsigned>(step.endVolume * 10.0f + 0.5f));
        line.assign(buffer);
        ++editor->writeLineIndex;
        return true;
    }
    return false;
}

bool PLAMIOSoundEffectEditor::writeSourceLine(std::string& line, void* arg)
{
    PLAMIOSoundEffectEditor* editor = static_cast<PLAMIOSoundEffectEditor*>(arg);
    char buffer[160];
    const uint8_t cursor = editor->writeLineIndex++;

    if (cursor == 0) { line.assign("#pragma once"); return true; }
    if (cursor == 1) { line.assign("#include \"PLAMIO.h\""); return true; }
    if (cursor == 2) { line.assign(""); return true; }
    if (cursor == 3)
    {
        std::snprintf(buffer, sizeof(buffer), "static const PLAMIO::Audio::SoundStep slot%uSteps[] =",
                      editor->writeSlot + 1);
        line.assign(buffer); return true;
    }
    if (cursor == 4) { line.assign("{"); return true; }

    const uint8_t stepLine = static_cast<uint8_t>(cursor - 5);
    if (stepLine < editor->stepCount)
    {
        const Audio::SoundStep& step = editor->steps[stepLine];
        std::snprintf(buffer, sizeof(buffer), "    { %u, %u, %u, %.1ff, %.1ff },",
                      step.startFrequency, step.endFrequency, step.durationMsec,
                      step.startVolume, step.endVolume);
        line.assign(buffer); return true;
    }

    const uint8_t footer = static_cast<uint8_t>(stepLine - editor->stepCount);
    switch (footer)
    {
    case 0: line.assign("};"); return true;
    case 1: line.assign(""); return true;
    case 2:
        std::snprintf(buffer, sizeof(buffer), "static const PLAMIO::Audio::Sound slot%uSound =",
                      editor->writeSlot + 1);
        line.assign(buffer); return true;
    case 3: line.assign("{"); return true;
    case 4:
        std::snprintf(buffer, sizeof(buffer), "    slot%uSteps,", editor->writeSlot + 1);
        line.assign(buffer); return true;
    case 5:
        std::snprintf(buffer, sizeof(buffer), "    static_cast<uint16_t>(sizeof(slot%uSteps) / sizeof(slot%uSteps[0]))",
                      editor->writeSlot + 1, editor->writeSlot + 1);
        line.assign(buffer); return true;
    case 6: line.assign("};"); return true;
    default: return false;
    }
}

void PLAMIOSoundEffectEditor::makeDataFileName(uint8_t slot, char* out, uint8_t outSize) const
{
    std::snprintf(out, outSize, "slot%u.dat", slot + 1);
}

void PLAMIOSoundEffectEditor::makeSourceFileName(uint8_t slot, char* out, uint8_t outSize) const
{
    std::snprintf(out, outSize, "slot%u.h", slot + 1);
}

void PLAMIOSoundEffectEditor::drawEditor(Graphics& graphics)
{
    graphics.fillScreen(COLOR_BG);
    graphics.fillRect(0, 0, 320, 38, COLOR_PANEL);
    graphics.drawString("SOUND EFFECT EDITOR", 10, 8, Graphics::WHITE, Graphics::SIZE_22B);

    char stepText[24];
    graphics.drawString("STEP", 275, 10, Graphics::WHITE, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
    std::snprintf(stepText, sizeof(stepText), "%u/%u", selectedStep + 1, stepCount);
    graphics.drawString(stepText, 310, 10, Graphics::WHITE, Graphics::SIZE_18,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);

    drawFrequencyGraph(graphics);
    drawFieldPanel(graphics);

    graphics.drawString(mode == Mode::VALUE_EDIT ? "LEFT/RIGHT:VALUE  Y:COARSE  A:OK  B:BACK"
                                                  : "A:EDIT  X:ADD  Y:COPY  B:DELETE  START:PLAY",
                        315, 32, Graphics::LIGHTGRAY, Graphics::SIZE_10,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
}

void PLAMIOSoundEffectEditor::drawFrequencyGraph(Graphics& graphics)
{
    graphics.fillRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, COLOR_PANEL);
    for (uint8_t i = 0; i <= 5; ++i)
    {
        const int16_t y = GRAPH_Y + static_cast<int16_t>(i * GRAPH_H / 5);
        graphics.drawLine(GRAPH_X, y, GRAPH_X + GRAPH_W, y, COLOR_GRID);
        char label[12];
        std::snprintf(label, sizeof(label), "%u", 5000 - i * 1000);
        graphics.drawString(label, GRAPH_X - 5, y, Graphics::LIGHTGRAY, Graphics::SIZE_10,
                            Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::MIDDLE);
    }

    uint32_t totalDuration = 0;
    for (uint8_t i = 0; i < stepCount; ++i) totalDuration += steps[i].durationMsec;
    if (totalDuration == 0) totalDuration = 1;

    uint32_t elapsed = 0;
    for (uint8_t i = 0; i < stepCount; ++i)
    {
        const Audio::SoundStep& step = steps[i];
        const int16_t x1 = GRAPH_X + static_cast<int16_t>((elapsed * GRAPH_W) / totalDuration);
        elapsed += step.durationMsec;
        const int16_t x2 = GRAPH_X + static_cast<int16_t>((elapsed * GRAPH_W) / totalDuration);
        const int16_t y1 = GRAPH_Y + GRAPH_H - static_cast<int16_t>((step.startFrequency * GRAPH_H) / FREQ_MAX);
        const int16_t y2 = GRAPH_Y + GRAPH_H - static_cast<int16_t>((step.endFrequency * GRAPH_H) / FREQ_MAX);

        if (i == selectedStep)
            graphics.drawRect(x1, GRAPH_Y + 1, static_cast<uint16_t>((x2 > x1 ? x2 - x1 : 1)), GRAPH_H - 2, 2, COLOR_CURSOR);
        graphics.drawLine(x1, y1, x2, y2, COLOR_LINE);
        graphics.fillCircle(x1, y1, 3, i == selectedStep ? COLOR_POINT : COLOR_LINE);
        graphics.fillCircle(x2, y2, 3, i == selectedStep ? COLOR_POINT : COLOR_LINE);
    }
}

void PLAMIOSoundEffectEditor::drawFieldPanel(Graphics& graphics)
{
    const Audio::SoundStep& step = steps[selectedStep];
    const char* labels[] = {"START FREQ", "END FREQ", "DURATION", "START VOL", "END VOL"};
    char values[5][24];
    std::snprintf(values[0], sizeof(values[0]), "%u Hz", step.startFrequency);
    std::snprintf(values[1], sizeof(values[1]), "%u Hz", step.endFrequency);
    std::snprintf(values[2], sizeof(values[2]), "%u ms", step.durationMsec);
    std::snprintf(values[3], sizeof(values[3]), "%.1f", step.startVolume);
    std::snprintf(values[4], sizeof(values[4]), "%.1f", step.endVolume);

    for (uint8_t i = 0; i < 5; ++i)
    {
        const int16_t y = PANEL_Y + i * ROW_H;
        if (i == static_cast<uint8_t>(selectedField))
            graphics.fillRoundRect(8, y - 1, 304, ROW_H, 3,
                                   mode == Mode::VALUE_EDIT ? COLOR_CURSOR : Graphics::Color::GRAY);
        const Graphics::Color color = (i == static_cast<uint8_t>(selectedField) && mode == Mode::VALUE_EDIT)
                                          ? COLOR_BG : Graphics::WHITE;
        graphics.drawString(labels[i], 15, y, color, Graphics::SIZE_13);
        graphics.drawString(values[i], 305, y, color, Graphics::SIZE_13,
                            Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOSoundEffectEditor::drawMainMenu(Graphics& graphics)
{
    graphics.fillRoundRect(56, 42, 208, 166, 8, COLOR_PANEL);
    graphics.drawRoundRect(56, 42, 208, 166, 8, 2, COLOR_BAR);
    graphics.drawString("SOUND MENU", 160, 54, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < MAIN_MENU_COUNT; ++i)
    {
        const int16_t y = 88 + i * 23;
        if (i == menuIndex) graphics.fillRoundRect(72, y - 2, 176, 21, 4, COLOR_CURSOR);
        graphics.drawString(MAIN_MENU_ITEMS[i], 160, y,
                            i == menuIndex ? COLOR_BG : Graphics::WHITE, Graphics::SIZE_18,
                            Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOSoundEffectEditor::drawSlotMenu(Graphics& graphics, bool saving)
{
    graphics.fillRoundRect(52, 50, 216, 145, 8, COLOR_PANEL);
    graphics.drawRoundRect(52, 50, 216, 145, 8, 2, COLOR_BAR);
    graphics.drawString(saving ? "SAVE SOUND" : "LOAD SOUND", 160, 62, Graphics::WHITE,
                        Graphics::SIZE_22B, Graphics::HorizontalAlign::CENTER,
                        Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < SLOT_COUNT; ++i)
    {
        const int16_t y = 100 + i * 27;
        if (i == menuIndex) graphics.fillRoundRect(68, y - 3, 184, 23, 4, COLOR_CURSOR);
        char text[40];
        if (slotExists[i])
            std::snprintf(text, sizeof(text), "SLOT %u   %u STEPS", i + 1, slotStepCount[i]);
        else
            std::snprintf(text, sizeof(text), "SLOT %u   EMPTY", i + 1);
        graphics.drawString(text, 160, y, i == menuIndex ? COLOR_BG : Graphics::WHITE,
                            Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER,
                            Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOSoundEffectEditor::drawConfirmation(Graphics& graphics, const char* title,
                                         const char* message, const char* yesText)
{
    graphics.fillRoundRect(35, 69, 250, 105, 8, COLOR_PANEL);
    graphics.drawRoundRect(35, 69, 250, 105, 8, 2, COLOR_CURSOR);
    graphics.drawString(title, 160, 82, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString(message, 160, 116, Graphics::LIGHTGRAY, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString(yesText, 80, 148, COLOR_CURSOR, Graphics::SIZE_13);
    graphics.drawString("B: CANCEL", 240, 148, Graphics::LIGHTGRAY, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
}

const char* PLAMIOSoundEffectEditor::fieldName(Field field)
{
    switch (field)
    {
    case Field::START_FREQ: return "START FREQ";
    case Field::END_FREQ: return "END FREQ";
    case Field::DURATION: return "DURATION";
    case Field::START_VOLUME: return "START VOL";
    case Field::END_VOLUME: return "END VOL";
    default: return "";
    }
}

float PLAMIOSoundEffectEditor::clampVolume(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    const int rounded = static_cast<int>(value * 10.0f + 0.5f);
    return static_cast<float>(rounded) / 10.0f;
}
