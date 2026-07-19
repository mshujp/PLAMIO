#include "PLAMIOMusicEditor.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace PLAMIO;

namespace
{
constexpr int16_t GRID_X = 47;
constexpr int16_t GRID_Y = 47;
constexpr int16_t CELL_W = 16;
constexpr int16_t CELL_H = 19;
constexpr int16_t GRID_W = 16 * CELL_W;
constexpr int16_t GRID_H = 8 * CELL_H;

constexpr Graphics::Color COLOR_BG = Graphics::rgb565(15, 20, 28);
constexpr Graphics::Color COLOR_PANEL = Graphics::rgb565(25, 33, 44);
constexpr Graphics::Color COLOR_GRID = Graphics::rgb565(60, 75, 92);
constexpr Graphics::Color COLOR_BAR = Graphics::rgb565(105, 125, 145);
constexpr Graphics::Color COLOR_NOTE = Graphics::rgb565(65, 205, 190);
constexpr Graphics::Color COLOR_CURSOR = Graphics::rgb565(255, 195, 70);
constexpr Graphics::Color COLOR_PLAYHEAD = Graphics::rgb565(255, 95, 110);

constexpr const char* MAIN_MENU_ITEMS[] = {
    "RESUME", "SAVE", "LOAD", "CLEAR", "SAMPLE RESET"
};
constexpr uint8_t MAIN_MENU_COUNT = sizeof(MAIN_MENU_ITEMS) / sizeof(MAIN_MENU_ITEMS[0]);
}

const char* PLAMIOMusicEditor::getId() const { return "plamio_musiceditor"; }
const char* PLAMIOMusicEditor::getName() const { return "PLAMIO Music Editor"; }
const char* PLAMIOMusicEditor::getMenuName() const { return "PLAMIO MUSIC EDITOR"; }
uint16_t PLAMIOMusicEditor::getLogicalScreenWidth() const { return Display::ILI9341_SCREEN_W; }
uint16_t PLAMIOMusicEditor::getLogicalScreenHeight() const { return Display::ILI9341_SCREEN_H; }
uint16_t PLAMIOMusicEditor::getTargetScreenWidth() const { return Display::ILI9341_SCREEN_W; }
uint16_t PLAMIOMusicEditor::getTargetScreenHeight() const { return Display::ILI9341_SCREEN_H; }

void PLAMIOMusicEditor::onInit(Storage& storage)
{
    cursorStep = 0;
    cursorPitch = 7;
    selectedLength = 1;
    bpm = BPM_DEFAULT;
    playing = false;
    playheadStep = 0;
    mode = Mode::EDIT;
    menuIndex = 0;
    selectedSlot = 0;
    loadSample();
    refreshSlotInfo(storage);
    captureSavedState();
    terminateAfterSave = false;
    terminateRequested = false;
    dirty = true;
}

Game::GameState PLAMIOMusicEditor::onUpdate(Input& input, Audio& audio,
                                      Storage& storage, float deltaSec)
{
    (void)deltaSec;
    const uint32_t now = Platform::getMsec();

    if (playing)
    {
        if (input.justPressed(Input::START) || input.justPressed(Input::B))
        {
            stopPlayback(audio);
            dirty = true;
            return GameState::RUNNING;
        }

        const uint32_t elapsed = static_cast<uint32_t>(now - playStartMsec);
        const uint32_t stepMsec = stepDurationMsec();
        const uint8_t nextStep = static_cast<uint8_t>(elapsed / stepMsec);
        if (nextStep >= STEP_COUNT)
            stopPlayback(audio);
        else if (nextStep != playheadStep)
            playheadStep = nextStep;
        dirty = true;
        return GameState::RUNNING;
    }

    if (mode == Mode::EDIT)
        updateEdit(input, audio);
    else
        updateMenu(input, audio, storage);

    return terminateRequested ? GameState::TERMINATE_REQUEST : GameState::RUNNING;
}

void PLAMIOMusicEditor::updateEdit(Input& input, Audio& audio)
{
    if (input.justPressed(Input::SELECT))
    {
        openMainMenu();
        audio.playSE(&Audio::SE::NO_1, 0.4f);
        return;
    }

    if (input.repeat(Input::LEFT))
    {
        if (input.pressed(Input::Y))
            bpm = static_cast<uint16_t>(bpm > BPM_MIN ? bpm - BPM_STEP : BPM_MIN);
        else if (cursorStep > 0)
            --cursorStep;
        dirty = true;
    }
    if (input.repeat(Input::RIGHT))
    {
        if (input.pressed(Input::Y))
            bpm = static_cast<uint16_t>(bpm < BPM_MAX ? bpm + BPM_STEP : BPM_MAX);
        else if (cursorStep + 1 < STEP_COUNT)
            ++cursorStep;
        dirty = true;
    }
    if (!input.pressed(Input::Y) && input.repeat(Input::UP) && cursorPitch > 0)
    {
        --cursorPitch;
        dirty = true;
    }
    if (!input.pressed(Input::Y) && input.repeat(Input::DOWN) && cursorPitch + 1 < PITCH_COUNT)
    {
        ++cursorPitch;
        dirty = true;
    }

    if (input.justPressed(Input::A))
    {
        placeNote();
        audio.playSE(&Audio::SE::NO_1, 0.5f);
        dirty = true;
    }
    if (input.justPressed(Input::B))
    {
        eraseAt(cursorStep);
        audio.playSE(&Audio::SE::NO_2, 0.4f);
        dirty = true;
    }
    if (input.justPressed(Input::X))
    {
        cycleLength();
        audio.playSE(&Audio::SE::NO_1, 0.4f);
        dirty = true;
    }
    if (input.justPressed(Input::START))
    {
        startPlayback(audio);
        dirty = true;
    }
}

void PLAMIOMusicEditor::updateMenu(Input& input, Audio& audio, Storage& storage)
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
                mode = Mode::LOAD_SLOTS; menuIndex = 0;
                break;
            case 3: mode = Mode::CONFIRM_CLEAR; break;
            case 4: loadSample(); mode = Mode::EDIT; break;
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

bool PLAMIOMusicEditor::onDraw(Graphics& graphics, bool requestFullRedraw)
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
        drawConfirmation(graphics, "OVERWRITE?", "Existing music will be replaced", "A: SAVE");
        break;
    case Mode::CONFIRM_LOAD:
        drawConfirmation(graphics, "LOAD MUSIC?", "Current music will be lost", "A: LOAD");
        break;
    case Mode::CONFIRM_EXIT:
        drawExitConfirmation(graphics);
        break;
    case Mode::CONFIRM_CLEAR:
        drawConfirmation(graphics, "CLEAR ALL?", "All notes will be removed", "A: CLEAR");
        break;
    default: break;
    }

    dirty = false;
    return true;
}

void PLAMIOMusicEditor::drawEditor(Graphics& graphics)
{
    graphics.fillScreen(COLOR_BG);
    graphics.fillRect(0, 0, 320, 38, COLOR_PANEL);
    graphics.drawString("MUSIC EDITOR", 10, 8, Graphics::WHITE, Graphics::SIZE_22B);

    char bpmText[24];
    std::snprintf(bpmText, sizeof(bpmText), "BPM %u", bpm);
    graphics.drawString(bpmText, 310, 10, Graphics::WHITE, Graphics::SIZE_18,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);

    for (uint8_t p = 0; p < PITCH_COUNT; ++p)
    {
        const int16_t y = GRID_Y + p * CELL_H;
        graphics.drawString(pitchName(p), 39, y + 2, Graphics::LIGHTGRAY, Graphics::SIZE_13,
                            Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
        graphics.drawLine(GRID_X, y, GRID_X + GRID_W, y, COLOR_GRID);
    }
    graphics.drawLine(GRID_X, GRID_Y + GRID_H, GRID_X + GRID_W, GRID_Y + GRID_H, COLOR_GRID);

    for (uint8_t s = 0; s <= STEP_COUNT; ++s)
    {
        const int16_t x = GRID_X + s * CELL_W;
        const Graphics::Color c = (s == 8) ? COLOR_BAR : ((s % 2 == 0) ? COLOR_GRID : COLOR_PANEL);
        graphics.drawLine(x, GRID_Y, x, GRID_Y + GRID_H, c);
    }

    for (uint8_t s = 0; s < STEP_COUNT; ++s)
    {
        if (grid[s].pitch < 0) continue;
        const int16_t x = GRID_X + s * CELL_W + 2;
        const int16_t y = GRID_Y + grid[s].pitch * CELL_H + 3;
        uint8_t visibleSteps = grid[s].lengthSteps;
        if (s + visibleSteps > STEP_COUNT) visibleSteps = static_cast<uint8_t>(STEP_COUNT - s);
        graphics.fillRoundRect(x, y, visibleSteps * CELL_W - 4, CELL_H - 6, 4, COLOR_NOTE);
    }

    const int16_t cursorX = GRID_X + cursorStep * CELL_W;
    const int16_t cursorY = GRID_Y + cursorPitch * CELL_H;
    graphics.drawRect(cursorX + 1, cursorY + 1, CELL_W - 2, CELL_H - 2, 2, COLOR_CURSOR);

    if (playing)
    {
        const int16_t playX = GRID_X + playheadStep * CELL_W;
        graphics.drawLine(playX, GRID_Y - 3, playX, GRID_Y + GRID_H + 3, COLOR_PLAYHEAD);
        graphics.drawString("PLAY", 310, 50, COLOR_PLAYHEAD, Graphics::Font::SIZE_25B,
                            Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
    }

    char info[48];
    std::snprintf(info, sizeof(info), "NOTE %s   STEP %u/16", lengthName(selectedLength), cursorStep + 1);
    graphics.drawString(info, 10, 204, Graphics::WHITE, Graphics::SIZE_13);
    graphics.drawString("A:PLACE   B:ERASE   X:LENGTH   SELECT:MENU", 160, 32,
                        Graphics::LIGHTGRAY, Graphics::SIZE_10,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
}

void PLAMIOMusicEditor::drawMainMenu(Graphics& graphics)
{
    graphics.fillRect(62, 43, 196, 170, COLOR_PANEL);
    graphics.drawRect(62, 43, 196, 170, 2, COLOR_CURSOR);
    graphics.drawString("MUSIC MENU", 160, 54, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < MAIN_MENU_COUNT; ++i)
    {
        if (i == menuIndex) graphics.fillRoundRect(82, 86 + i * 23, 156, 20, 4, COLOR_NOTE);
        graphics.drawString(MAIN_MENU_ITEMS[i], 160, 88 + i * 23,
                            i == menuIndex ? COLOR_BG : Graphics::WHITE, Graphics::SIZE_18,
                            Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOMusicEditor::drawSlotMenu(Graphics& graphics, bool saving)
{
    graphics.fillRect(54, 55, 212, 142, COLOR_PANEL);
    graphics.drawRect(54, 55, 212, 142, 2, COLOR_CURSOR);
    graphics.drawString(saving ? "SAVE MUSIC" : "LOAD MUSIC", 160, 66,
                        Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);

    for (uint8_t i = 0; i < SLOT_COUNT; ++i)
    {
        const int16_t y = 103 + i * 27;
        if (i == menuIndex) graphics.fillRoundRect(72, y - 3, 176, 23, 4, COLOR_NOTE);
        char text[40];
        if (slotExists[i])
            std::snprintf(text, sizeof(text), "SLOT %u   BPM %u", i + 1, slotBpm[i]);
        else
            std::snprintf(text, sizeof(text), "SLOT %u   EMPTY", i + 1);
        graphics.drawString(text, 160, y,
                            i == menuIndex ? COLOR_BG : (slotExists[i] ? Graphics::WHITE : Graphics::GRAY),
                            Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER,
                            Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOMusicEditor::drawConfirmation(Graphics& graphics, const char* title,
                                   const char* message, const char* yesText)
{
    graphics.fillRect(38, 77, 244, 94, COLOR_PANEL);
    graphics.drawRect(38, 77, 244, 94, 2, COLOR_CURSOR);
    graphics.drawString(title, 160, 88, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString(message, 160, 119, Graphics::LIGHTGRAY, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString(yesText, 72, 146, COLOR_NOTE, Graphics::SIZE_13);
    graphics.drawString("B: CANCEL", 248, 146, Graphics::LIGHTGRAY, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
}

void PLAMIOMusicEditor::drawExitConfirmation(Graphics& graphics)
{
    static const char* ITEMS[] = {"SAVE", "DON'T SAVE", "CANCEL"};
    graphics.fillRect(42, 55, 236, 142, COLOR_PANEL);
    graphics.drawRect(42, 55, 236, 142, 2, COLOR_CURSOR);
    graphics.drawString("UNSAVED CHANGES", 160, 66, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < 3; ++i)
    {
        if (i == menuIndex) graphics.fillRoundRect(70, 101 + i * 25, 180, 21, 4, COLOR_NOTE);
        graphics.drawString(ITEMS[i], 160, 103 + i * 25,
                            i == menuIndex ? COLOR_BG : Graphics::WHITE, Graphics::SIZE_18,
                            Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    }
}

Game::TerminateResponse PLAMIOMusicEditor::onRequestTerminate()
{
    if (!hasUnsavedChanges()) return TerminateResponse::ACCEPT;
    mode = Mode::CONFIRM_EXIT;
    menuIndex = 0;
    terminateAfterSave = false;
    dirty = true;
    return TerminateResponse::REJECT;
}

void PLAMIOMusicEditor::captureSavedState()
{
    std::memcpy(savedGrid, grid, sizeof(grid));
    savedBpm = bpm;
}

bool PLAMIOMusicEditor::hasUnsavedChanges() const
{
    return bpm != savedBpm || std::memcmp(savedGrid, grid, sizeof(grid)) != 0;
}

void PLAMIOMusicEditor::onTerminate(Storage& storage)
{
    (void)storage;
}

void PLAMIOMusicEditor::openMainMenu()
{
    mode = Mode::MAIN_MENU;
    menuIndex = 0;
    dirty = true;
}

void PLAMIOMusicEditor::moveMenuSelection(int8_t direction, uint8_t itemCount)
{
    if (direction < 0)
        menuIndex = menuIndex == 0 ? static_cast<uint8_t>(itemCount - 1) : static_cast<uint8_t>(menuIndex - 1);
    else
        menuIndex = static_cast<uint8_t>((menuIndex + 1) % itemCount);
    dirty = true;
}

void PLAMIOMusicEditor::refreshSlotInfo(Storage& storage)
{
    for (uint8_t i = 0; i < SLOT_COUNT; ++i)
    {
        slotExists[i] = loadSlot(storage, i, false);
        if (!slotExists[i]) slotBpm[i] = 0;
    }
}

bool PLAMIOMusicEditor::saveSlot(Storage& storage, uint8_t slot)
{
    char dataName[20];
    char sourceName[20];
    makeDataFileName(slot, dataName, sizeof(dataName));
    makeSourceFileName(slot, sourceName, sizeof(sourceName));

    writeSlot = slot;
    writeLineIndex = 0;
    const bool dataOk = storage.writeUserFile(getId(), dataName, &PLAMIOMusicEditor::writeDataLine, this);
    writeLineIndex = 0;
    const bool sourceOk = storage.writeUserFile(getId(), sourceName, &PLAMIOMusicEditor::writeSourceLine, this);
    const bool success = dataOk && sourceOk;
    if (success) captureSavedState();
    return success;
}

bool PLAMIOMusicEditor::loadSlot(Storage& storage, uint8_t slot, bool apply)
{
    char dataName[20];
    makeDataFileName(slot, dataName, sizeof(dataName));
    ReadContext context{this, apply, false, 0, BPM_DEFAULT};
    if (apply) clearAll();
    const bool readOk = storage.readUserFile(getId(), dataName, &PLAMIOMusicEditor::readSlotLine, &context);
    if (!readOk || !context.valid) return false;

    slotBpm[slot] = context.readBpm;
    if (apply)
    {
        bpm = context.readBpm;
        cursorStep = 0;
        cursorPitch = 7;
        captureSavedState();
    }
    return true;
}

bool PLAMIOMusicEditor::readSlotLine(const char* line, void* arg)
{
    ReadContext* context = static_cast<ReadContext*>(arg);
    if (std::strncmp(line, "BPM=", 4) == 0)
    {
        const int value = std::atoi(line + 4);
        if (value < BPM_MIN || value > BPM_MAX) return false;
        context->readBpm = static_cast<uint16_t>(value);
        context->valid = true;
        return true;
    }

    unsigned step = 0;
    int pitch = -1;
    unsigned length = 0;
    if (std::sscanf(line, "N=%u,%d,%u", &step, &pitch, &length) == 3)
    {
        if (step >= STEP_COUNT || pitch < -1 || pitch >= PITCH_COUNT ||
            (length != 0 && length != 1 && length != 2 && length != 4))
            return false;
        if (context->apply)
        {
            context->editor->grid[step].pitch = static_cast<int8_t>(pitch);
            context->editor->grid[step].lengthSteps = static_cast<uint8_t>(length);
        }
        ++context->noteIndex;
    }
    return true;
}

bool PLAMIOMusicEditor::writeDataLine(std::string& line, void* arg)
{
    PLAMIOMusicEditor* editor = static_cast<PLAMIOMusicEditor*>(arg);
    char buffer[48];
    if (editor->writeLineIndex == 0)
    {
        std::snprintf(buffer, sizeof(buffer), "BPM=%u", editor->bpm);
        line.assign(buffer);
        ++editor->writeLineIndex;
        return true;
    }

    const uint8_t step = static_cast<uint8_t>(editor->writeLineIndex - 1);
    if (step < STEP_COUNT)
    {
        std::snprintf(buffer, sizeof(buffer), "N=%u,%d,%u", step,
                      editor->grid[step].pitch, editor->grid[step].lengthSteps);
        line.assign(buffer);
        ++editor->writeLineIndex;
        return true;
    }
    return false;
}

bool PLAMIOMusicEditor::writeSourceLine(std::string& line, void* arg)
{
    PLAMIOMusicEditor* editor = static_cast<PLAMIOMusicEditor*>(arg);
    char buffer[128];
    uint8_t& index = editor->writeLineIndex;

    if (index == 0) { line.assign("#pragma once"); ++index; return true; }
    if (index == 1) { line.assign("#include \"PLAMIO.h\""); ++index; return true; }
    if (index == 2) { line.assign(""); ++index; return true; }
    if (index == 3)
    {
        std::snprintf(buffer, sizeof(buffer), "static const PLAMIO::Audio::ToneNote slot%uNotes[] =", editor->writeSlot + 1);
        line.assign(buffer); ++index; return true;
    }
    if (index == 4) { line.assign("{"); ++index; return true; }

    uint8_t step = 0;
    uint8_t emitted = 0;
    while (step < STEP_COUNT)
    {
        const GridNote& note = editor->grid[step];
        if (emitted == index - 5)
        {
            if (note.pitch >= 0)
                std::snprintf(buffer, sizeof(buffer),
                              "    { PLAMIO::Audio::ToneNote::%s, PLAMIO::Audio::ToneNote::%s },",
                              pitchName(static_cast<uint8_t>(note.pitch)), durationSourceName(note.lengthSteps));
            else
                std::snprintf(buffer, sizeof(buffer),
                              "    { PLAMIO::Audio::ToneNote::REST, PLAMIO::Audio::ToneNote::E },");
            line.assign(buffer); ++index; return true;
        }
        step = static_cast<uint8_t>(step + (note.pitch >= 0 && note.lengthSteps > 0 ? note.lengthSteps : 1));
        ++emitted;
    }

    const uint8_t footer = static_cast<uint8_t>(index - 5 - emitted);
    if (footer == 0) { line.assign("};"); ++index; return true; }
    if (footer == 1) { line.assign(""); ++index; return true; }
    if (footer == 2)
    {
        std::snprintf(buffer, sizeof(buffer), "static const PLAMIO::Audio::Music slot%uMusic =", editor->writeSlot + 1);
        line.assign(buffer); ++index; return true;
    }
    if (footer == 3) { line.assign("{"); ++index; return true; }
    if (footer == 4)
    {
        std::snprintf(buffer, sizeof(buffer), "    slot%uNotes,", editor->writeSlot + 1);
        line.assign(buffer); ++index; return true;
    }
    if (footer == 5)
    {
        std::snprintf(buffer, sizeof(buffer), "    %u,", editor->bpm);
        line.assign(buffer); ++index; return true;
    }
    if (footer == 6)
    {
        std::snprintf(buffer, sizeof(buffer),
                      "    static_cast<uint16_t>(sizeof(slot%uNotes) / sizeof(slot%uNotes[0])),",
                      editor->writeSlot + 1, editor->writeSlot + 1);
        line.assign(buffer); ++index; return true;
    }
    if (footer == 7) { line.assign("    1,"); ++index; return true; }
    if (footer == 8) { line.assign("    0.7f"); ++index; return true; }
    if (footer == 9) { line.assign("};"); ++index; return true; }
    return false;
}

void PLAMIOMusicEditor::makeDataFileName(uint8_t slot, char* out, uint8_t outSize) const
{
    std::snprintf(out, outSize, "slot%u.dat", slot + 1);
}

void PLAMIOMusicEditor::makeSourceFileName(uint8_t slot, char* out, uint8_t outSize) const
{
    std::snprintf(out, outSize, "slot%u.h", slot + 1);
}

void PLAMIOMusicEditor::loadSample()
{
    clearAll();
    const int8_t pitches[8] = {7, 6, 5, 3, 0, 3, 5, 7};
    const uint8_t steps[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    for (uint8_t i = 0; i < 8; ++i)
    {
        grid[steps[i]].pitch = pitches[i];
        grid[steps[i]].lengthSteps = 2;
    }
    bpm = BPM_DEFAULT;
}

void PLAMIOMusicEditor::clearAll()
{
    for (uint8_t i = 0; i < STEP_COUNT; ++i)
    {
        grid[i].pitch = -1;
        grid[i].lengthSteps = 0;
    }
}

void PLAMIOMusicEditor::placeNote()
{
    removeOverlaps(cursorStep, selectedLength);
    grid[cursorStep].pitch = static_cast<int8_t>(cursorPitch);
    grid[cursorStep].lengthSteps = selectedLength;
}

void PLAMIOMusicEditor::eraseAt(uint8_t step)
{
    const int8_t start = findCoveringNoteStart(step);
    if (start >= 0)
    {
        grid[start].pitch = -1;
        grid[start].lengthSteps = 0;
    }
}

void PLAMIOMusicEditor::cycleLength()
{
    selectedLength = selectedLength == 1 ? 2 : (selectedLength == 2 ? 4 : 1);
    const int8_t start = findCoveringNoteStart(cursorStep);
    if (start >= 0)
    {
        const int8_t pitch = grid[start].pitch;
        grid[start].pitch = -1;
        grid[start].lengthSteps = 0;
        removeOverlaps(static_cast<uint8_t>(start), selectedLength);
        grid[start].pitch = pitch;
        grid[start].lengthSteps = selectedLength;
    }
}

void PLAMIOMusicEditor::removeOverlaps(uint8_t startStep, uint8_t lengthSteps)
{
    uint8_t endStep = static_cast<uint8_t>(startStep + lengthSteps);
    if (endStep > STEP_COUNT) endStep = STEP_COUNT;

    for (uint8_t s = 0; s < STEP_COUNT; ++s)
    {
        if (grid[s].pitch < 0) continue;
        uint8_t existingEnd = static_cast<uint8_t>(s + grid[s].lengthSteps);
        if (existingEnd > STEP_COUNT) existingEnd = STEP_COUNT;
        if (s < endStep && existingEnd > startStep)
        {
            grid[s].pitch = -1;
            grid[s].lengthSteps = 0;
        }
    }
}

int8_t PLAMIOMusicEditor::findCoveringNoteStart(uint8_t step) const
{
    for (uint8_t s = 0; s < STEP_COUNT; ++s)
    {
        if (grid[s].pitch < 0) continue;
        const uint8_t end = static_cast<uint8_t>(s + grid[s].lengthSteps);
        if (step >= s && step < end) return static_cast<int8_t>(s);
    }
    return -1;
}

void PLAMIOMusicEditor::buildPlayback()
{
    uint8_t count = 0;
    uint8_t step = 0;
    while (step < STEP_COUNT && count < MAX_PLAYBACK_NOTES)
    {
        if (grid[step].pitch >= 0)
        {
            uint8_t length = grid[step].lengthSteps;
            if (step + length > STEP_COUNT) length = static_cast<uint8_t>(STEP_COUNT - step);
            playbackNotes[count++] = Audio::ToneNote(pitchFrequency(grid[step].pitch), durationForSteps(length));
            step = static_cast<uint8_t>(step + length);
        }
        else
        {
            playbackNotes[count++] = Audio::ToneNote(Audio::ToneNote::REST, Audio::ToneNote::E);
            ++step;
        }
    }

    playbackMusic.notes = playbackNotes;
    playbackMusic.bpm = bpm;
    playbackMusic.noteCount = count;
    playbackMusic.playCount = 1;
    playbackMusic.gain = 0.7f;
}

void PLAMIOMusicEditor::startPlayback(Audio& audio)
{
    buildPlayback();
    audio.playMusic(&playbackMusic);
    playing = true;
    playStartMsec = Platform::getMsec();
    playheadStep = 0;
}

void PLAMIOMusicEditor::stopPlayback(Audio& audio)
{
    audio.stopMusic();
    playing = false;
    playheadStep = 0;
}

uint32_t PLAMIOMusicEditor::stepDurationMsec() const
{
    return 30000u / bpm;
}

uint16_t PLAMIOMusicEditor::pitchFrequency(uint8_t pitch)
{
    static constexpr uint16_t frequencies[PITCH_COUNT] = {
        Audio::ToneNote::C5, Audio::ToneNote::B4, Audio::ToneNote::A4, Audio::ToneNote::G4,
        Audio::ToneNote::F4, Audio::ToneNote::E4, Audio::ToneNote::D4, Audio::ToneNote::C4
    };
    return frequencies[pitch < PITCH_COUNT ? pitch : 0];
}

Audio::ToneNote::Duration PLAMIOMusicEditor::durationForSteps(uint8_t steps)
{
    if (steps >= 4) return Audio::ToneNote::H;
    if (steps >= 2) return Audio::ToneNote::Q;
    return Audio::ToneNote::E;
}

const char* PLAMIOMusicEditor::pitchName(uint8_t pitch)
{
    static const char* names[PITCH_COUNT] = {"C5", "B4", "A4", "G4", "F4", "E4", "D4", "C4"};
    return names[pitch < PITCH_COUNT ? pitch : 0];
}

const char* PLAMIOMusicEditor::lengthName(uint8_t steps)
{
    if (steps == 4) return "1/2";
    if (steps == 2) return "1/4";
    return "1/8";
}

const char* PLAMIOMusicEditor::durationSourceName(uint8_t steps)
{
    if (steps >= 4) return "H";
    if (steps >= 2) return "Q";
    return "E";
}
