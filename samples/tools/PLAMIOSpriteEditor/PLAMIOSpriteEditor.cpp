#include "PLAMIOSpriteEditor.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace PLAMIO;

namespace
{
constexpr int16_t CANVAS_X = 12;
constexpr int16_t CANVAS_Y = 39;
constexpr int16_t CELL = 10;
constexpr int16_t CANVAS_SIZE = 16 * CELL;
constexpr int16_t PREVIEW_X = 217;
constexpr int16_t PREVIEW_Y = 60;

constexpr Graphics::Color COLOR_BG = Graphics::rgb565(15, 20, 28);
constexpr Graphics::Color COLOR_PANEL = Graphics::rgb565(25, 33, 44);
constexpr Graphics::Color COLOR_GRID = Graphics::rgb565(55, 68, 82);
constexpr Graphics::Color COLOR_TEXT_DIM = Graphics::rgb565(145, 160, 175);
constexpr Graphics::Color COLOR_CURSOR = Graphics::rgb565(255, 195, 70);
constexpr Graphics::Color COLOR_ACCENT = Graphics::rgb565(65, 205, 190);

constexpr const char* MAIN_MENU_ITEMS[] = {
    "RESUME", "SAVE", "LOAD", "CLEAR", "SAMPLE RESET", "TRANSFORM"
};
constexpr uint8_t MAIN_MENU_COUNT = sizeof(MAIN_MENU_ITEMS) / sizeof(MAIN_MENU_ITEMS[0]);

constexpr const char* TRANSFORM_ITEMS[] = {
    "FLIP X", "FLIP Y", "ROTATE 90"
};
constexpr uint8_t TRANSFORM_COUNT = sizeof(TRANSFORM_ITEMS) / sizeof(TRANSFORM_ITEMS[0]);
}

const char* PLAMIOSpriteEditor::getId() const { return "plamio_spriteeditor"; }
const char* PLAMIOSpriteEditor::getName() const { return "PLAMIO Sprite Editor"; }
const char* PLAMIOSpriteEditor::getMenuName() const { return "PLAMIO SPRITE EDITOR"; }
uint16_t PLAMIOSpriteEditor::getLogicalScreenWidth() const { return Display::ILI9341_SCREEN_W; }
uint16_t PLAMIOSpriteEditor::getLogicalScreenHeight() const { return Display::ILI9341_SCREEN_H; }
uint16_t PLAMIOSpriteEditor::getTargetScreenWidth() const { return Display::ILI9341_SCREEN_W; }
uint16_t PLAMIOSpriteEditor::getTargetScreenHeight() const { return Display::ILI9341_SCREEN_H; }

void PLAMIOSpriteEditor::onInit(Storage& storage)
{
    cursorX = 0;
    cursorY = 0;
    selectedColor = 1;
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

Game::GameState PLAMIOSpriteEditor::onUpdate(Input& input, Audio& audio,
                                        Storage& storage, float deltaSec)
{
    (void)deltaSec;
    if (mode == Mode::EDIT || mode == Mode::PREVIEW)
        updateEdit(input, audio);
    else
        updateMenu(input, audio, storage);
    return terminateRequested ? GameState::TERMINATE_REQUEST : GameState::RUNNING;
}

void PLAMIOSpriteEditor::updateEdit(Input& input, Audio& audio)
{
    if (input.justPressed(Input::SELECT))
    {
        openMainMenu();
        audio.playSE(&Audio::SE::NO_1, 0.35f);
        return;
    }

    if (input.justPressed(Input::START))
    {
        mode = (mode == Mode::PREVIEW) ? Mode::EDIT : Mode::PREVIEW;
        audio.playSE(&Audio::SE::NO_1, 0.35f);
        dirty = true;
        return;
    }

    if (mode == Mode::PREVIEW)
        return;

    if (input.repeat(Input::LEFT))  { cursorX = (cursorX == 0) ? SPRITE_W - 1 : cursorX - 1; dirty = true; }
    if (input.repeat(Input::RIGHT)) { cursorX = (cursorX + 1) % SPRITE_W; dirty = true; }
    if (input.repeat(Input::UP))    { cursorY = (cursorY == 0) ? SPRITE_H - 1 : cursorY - 1; dirty = true; }
    if (input.repeat(Input::DOWN))  { cursorY = (cursorY + 1) % SPRITE_H; dirty = true; }

    if (input.pressed(Input::A))
    {
        paintPixel();
        dirty = true;
    }
    if (input.pressed(Input::B))
    {
        erasePixel();
        dirty = true;
    }
    if (input.justPressed(Input::X))
    {
        cycleColor(1);
        audio.playSE(&Audio::SE::NO_1, 0.25f);
        dirty = true;
    }
    if (input.justPressed(Input::Y))
    {
        pickColor();
        audio.playSE(&Audio::SE::NO_3, 0.3f);
        dirty = true;
    }
}

void PLAMIOSpriteEditor::updateMenu(Input& input, Audio& audio, Storage& storage)
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
    if (mode == Mode::SAVE_SLOTS || mode == Mode::LOAD_SLOTS) itemCount = SLOT_COUNT;
    if (mode == Mode::TRANSFORM_MENU) itemCount = TRANSFORM_COUNT;

    if (mode == Mode::MAIN_MENU || mode == Mode::SAVE_SLOTS ||
        mode == Mode::LOAD_SLOTS || mode == Mode::TRANSFORM_MENU)
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
            audio.playSE(&Audio::SE::NO_2, 0.35f);
            dirty = true;
            return;
        }
        if (!input.justPressed(Input::A)) return;

        audio.playSE(&Audio::SE::NO_1, 0.35f);
        if (mode == Mode::MAIN_MENU)
        {
            switch (menuIndex)
            {
            case 0: mode = Mode::EDIT; break;
            case 1:
                if (!storage.isAvailable())
                {
                    audio.playSE(&Audio::SE::NO_1, 0.35f);
                    break;
                }
                mode = Mode::SAVE_SLOTS;
                menuIndex = 0;
                break;
            case 2:
                if (!storage.isAvailable())
                {
                    audio.playSE(&Audio::SE::NO_1, 0.35f);
                    break;
                }
                mode = Mode::LOAD_SLOTS;
                menuIndex = 0;
                break;
            case 3: mode = Mode::CONFIRM_CLEAR; break;
            case 4: loadSample(); mode = Mode::EDIT; break;
            case 5: mode = Mode::TRANSFORM_MENU; menuIndex = 0; break;
            default: break;
            }
        }
        else if (mode == Mode::SAVE_SLOTS)
        {
            selectedSlot = menuIndex;
            if (slotExists[selectedSlot]) mode = Mode::CONFIRM_OVERWRITE;
            else
            {
                const bool ok = saveSlot(storage, selectedSlot);
                refreshSlotInfo(storage);
                audio.playSE(ok ? &Audio::SE::NO_3 : &Audio::SE::NO_2, 0.5f);
                if (ok && terminateAfterSave)
                    terminateRequested = true;
                else
                    mode = Mode::EDIT;
            }
        }
        else if (mode == Mode::LOAD_SLOTS)
        {
            selectedSlot = menuIndex;
            if (slotExists[selectedSlot]) mode = Mode::CONFIRM_LOAD;
            else audio.playSE(&Audio::SE::NO_2, 0.5f);
        }
        else
        {
            if (menuIndex == 0) flipHorizontal();
            else if (menuIndex == 1) flipVertical();
            else rotate90();
            mode = Mode::EDIT;
            audio.playSE(&Audio::SE::NO_3, 0.45f);
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
        dirty = true;
        return;
    }
    if (!input.justPressed(Input::A)) return;

    bool ok = true;
    if (mode == Mode::CONFIRM_OVERWRITE)
    {
        ok = saveSlot(storage, selectedSlot);
        refreshSlotInfo(storage);
    }
    else if (mode == Mode::CONFIRM_LOAD)
        ok = loadSlot(storage, selectedSlot, true);
    else if (mode == Mode::CONFIRM_CLEAR)
        clearAll();

    audio.playSE(ok ? &Audio::SE::NO_3 : &Audio::SE::NO_2, 0.5f);
    if (ok && terminateAfterSave && mode == Mode::CONFIRM_OVERWRITE)
        terminateRequested = true;
    else
        mode = Mode::EDIT;
    dirty = true;
}

bool PLAMIOSpriteEditor::onDraw(Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty) return false;

    drawEditor(graphics);
    if (mode == Mode::PREVIEW) drawPreview(graphics);
    else if (mode == Mode::MAIN_MENU) drawMainMenu(graphics);
    else if (mode == Mode::SAVE_SLOTS) drawSlotMenu(graphics, true);
    else if (mode == Mode::LOAD_SLOTS) drawSlotMenu(graphics, false);
    else if (mode == Mode::TRANSFORM_MENU) drawTransformMenu(graphics);
    else if (mode == Mode::CONFIRM_OVERWRITE) drawConfirmation(graphics, "OVERWRITE?", "SLOT DATA WILL BE LOST", "A: SAVE");
    else if (mode == Mode::CONFIRM_LOAD) drawConfirmation(graphics, "LOAD?", "CURRENT SPRITE WILL BE LOST", "A: LOAD");
    else if (mode == Mode::CONFIRM_CLEAR) drawConfirmation(graphics, "CLEAR?", "ERASE ALL PIXELS", "A: CLEAR");
    else if (mode == Mode::CONFIRM_EXIT) drawExitConfirmation(graphics);

    dirty = false;
    return true;
}

void PLAMIOSpriteEditor::drawExitConfirmation(Graphics& graphics)
{
    static const char* ITEMS[] = {"SAVE", "DON'T SAVE", "CANCEL"};
    graphics.fillRect(42, 55, 236, 142, COLOR_PANEL);
    graphics.drawRect(42, 55, 236, 142, 2, COLOR_CURSOR);
    graphics.drawString("UNSAVED CHANGES", 160, 66, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < 3; ++i)
    {
        if (i == menuIndex) graphics.fillRoundRect(70, 101 + i * 25, 180, 21, 4, COLOR_ACCENT);
        graphics.drawString(ITEMS[i], 160, 103 + i * 25,
                            i == menuIndex ? COLOR_BG : Graphics::WHITE, Graphics::SIZE_18,
                            Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    }
}

Game::TerminateResponse PLAMIOSpriteEditor::onRequestTerminate()
{
    if (!hasUnsavedChanges()) return TerminateResponse::ACCEPT;
    mode = Mode::CONFIRM_EXIT;
    menuIndex = 0;
    terminateAfterSave = false;
    dirty = true;
    return TerminateResponse::REJECT;
}

void PLAMIOSpriteEditor::captureSavedState()
{
    std::memcpy(savedPixels, pixels, sizeof(pixels));
}

bool PLAMIOSpriteEditor::hasUnsavedChanges() const
{
    return std::memcmp(savedPixels, pixels, sizeof(pixels)) != 0;
}

void PLAMIOSpriteEditor::onTerminate(Storage& storage) { (void)storage; }

void PLAMIOSpriteEditor::loadSample()
{
    clearAll();
    static const uint8_t sample[16][16] = {
        {0,0,0,0,0,0,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,2,2,6,6,6,6,2,2,0,0,0,0},
        {0,0,0,2,6,6,6,6,6,6,6,6,2,0,0,0},
        {0,0,2,6,6,15,6,6,6,15,6,6,6,2,0,0},
        {0,0,2,6,6,15,6,6,6,15,6,6,6,2,0,0},
        {0,2,6,6,6,6,6,6,6,6,6,6,6,6,2,0},
        {0,2,6,6,6,6,6,6,6,6,6,6,6,6,2,0},
        {0,2,6,6,6,6,15,15,15,15,6,6,6,6,2,0},
        {0,2,6,6,6,6,6,15,15,6,6,6,6,6,2,0},
        {0,0,2,6,6,6,6,6,6,6,6,6,6,2,0,0},
        {0,0,2,2,6,6,6,6,6,6,6,6,2,2,0,0},
        {0,0,0,0,2,2,6,6,6,6,2,2,0,0,0,0},
        {0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0},
        {0,0,0,0,2,2,0,0,0,0,2,2,0,0,0,0},
        {0,0,0,2,2,0,0,0,0,0,0,2,2,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
    for (uint8_t y = 0; y < SPRITE_H; ++y)
        for (uint8_t x = 0; x < SPRITE_W; ++x)
            pixels[y * SPRITE_W + x] = sample[y][x];
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::clearAll()
{
    memset(pixels, 0, sizeof(pixels));
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::paintPixel()
{
    pixels[cursorY * SPRITE_W + cursorX] = selectedColor;
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::erasePixel()
{
    pixels[cursorY * SPRITE_W + cursorX] = 0;
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::pickColor()
{
    const uint8_t value = pixels[cursorY * SPRITE_W + cursorX];
    if (value != 0) selectedColor = value;
}

void PLAMIOSpriteEditor::cycleColor(int8_t direction)
{
    int16_t value = static_cast<int16_t>(selectedColor) + direction;
    if (value < 1) value = PALETTE_SIZE - 1;
    if (value >= PALETTE_SIZE) value = 1;
    selectedColor = static_cast<uint8_t>(value);
}

void PLAMIOSpriteEditor::rebuildRenderPixels()
{
    for (uint16_t i = 0; i < PIXEL_COUNT; ++i)
        renderPixels[i] = static_cast<uint16_t>(paletteColor(pixels[i]));
}

void PLAMIOSpriteEditor::flipHorizontal()
{
    for (uint8_t y = 0; y < SPRITE_H; ++y)
        for (uint8_t x = 0; x < SPRITE_W / 2; ++x)
        {
            const uint16_t a = y * SPRITE_W + x;
            const uint16_t b = y * SPRITE_W + (SPRITE_W - 1 - x);
            const uint8_t t = pixels[a]; pixels[a] = pixels[b]; pixels[b] = t;
        }
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::flipVertical()
{
    for (uint8_t y = 0; y < SPRITE_H / 2; ++y)
        for (uint8_t x = 0; x < SPRITE_W; ++x)
        {
            const uint16_t a = y * SPRITE_W + x;
            const uint16_t b = (SPRITE_H - 1 - y) * SPRITE_W + x;
            const uint8_t t = pixels[a]; pixels[a] = pixels[b]; pixels[b] = t;
        }
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::rotate90()
{
    uint8_t temp[PIXEL_COUNT];
    memcpy(temp, pixels, sizeof(temp));
    for (uint8_t y = 0; y < SPRITE_H; ++y)
        for (uint8_t x = 0; x < SPRITE_W; ++x)
            pixels[y * SPRITE_W + x] = temp[(SPRITE_H - 1 - x) * SPRITE_W + y];
    rebuildRenderPixels();
}

void PLAMIOSpriteEditor::openMainMenu()
{
    mode = Mode::MAIN_MENU;
    menuIndex = 0;
    dirty = true;
}

void PLAMIOSpriteEditor::moveMenuSelection(int8_t direction, uint8_t itemCount)
{
    int16_t next = static_cast<int16_t>(menuIndex) + direction;
    if (next < 0) next = itemCount - 1;
    if (next >= itemCount) next = 0;
    menuIndex = static_cast<uint8_t>(next);
    dirty = true;
}

void PLAMIOSpriteEditor::makeDataFileName(uint8_t slot, char* out, uint8_t outSize) const
{
    snprintf(out, outSize, "slot%u.dat", static_cast<unsigned>(slot + 1));
}
void PLAMIOSpriteEditor::makeSourceFileName(uint8_t slot, char* out, uint8_t outSize) const
{
    snprintf(out, outSize, "slot%u.h", static_cast<unsigned>(slot + 1));
}

void PLAMIOSpriteEditor::refreshSlotInfo(Storage& storage)
{
    for (uint8_t i = 0; i < SLOT_COUNT; ++i)
        slotExists[i] = loadSlot(storage, i, false);
}

bool PLAMIOSpriteEditor::saveSlot(Storage& storage, uint8_t slot)
{
    writeSlot = slot;
    writeLineIndex = 0;
    char dataName[16];
    char sourceName[16];
    makeDataFileName(slot, dataName, sizeof(dataName));
    makeSourceFileName(slot, sourceName, sizeof(sourceName));
    const bool dataOk = storage.writeUserFile(getId(), dataName, &PLAMIOSpriteEditor::writeDataLine, this);
    writeLineIndex = 0;
    const bool sourceOk = storage.writeUserFile(getId(), sourceName, &PLAMIOSpriteEditor::writeSourceLine, this);
    const bool success = dataOk && sourceOk;
    if (success) captureSavedState();
    return success;
}

bool PLAMIOSpriteEditor::loadSlot(Storage& storage, uint8_t slot, bool apply)
{
    char fileName[16];
    makeDataFileName(slot, fileName, sizeof(fileName));
    ReadContext context{this, apply, true, 0};
    const bool opened = storage.readUserFile(getId(), fileName, &PLAMIOSpriteEditor::readSlotLine, &context);
    if (opened && context.valid && context.pixelIndex == PIXEL_COUNT && apply)
    {
        rebuildRenderPixels();
        cursorX = cursorY = 0;
        captureSavedState();
    }
    return opened && context.valid && context.pixelIndex == PIXEL_COUNT;
}

bool PLAMIOSpriteEditor::readSlotLine(const char* line, void* arg)
{
    ReadContext* context = static_cast<ReadContext*>(arg);
    if (!context || !line) return false;
    if (strncmp(line, "SIZE=16x16", 10) == 0) return true;
    if (strncmp(line, "ROW=", 4) != 0 || context->pixelIndex >= PIXEL_COUNT)
    {
        context->valid = false;
        return false;
    }

    const char* cursor = line + 4;
    for (uint8_t x = 0; x < SPRITE_W; ++x)
    {
        char* next = nullptr;
        const long value = strtol(cursor, &next, 10);
        if (next == cursor || value < 0 || value >= PALETTE_SIZE)
        {
            context->valid = false;
            return false;
        }
        if (context->apply)
            context->editor->pixels[context->pixelIndex] = static_cast<uint8_t>(value);
        ++context->pixelIndex;
        if (x + 1 < SPRITE_W)
        {
            if (*next != ',')
            {
                context->valid = false;
                return false;
            }
            cursor = next + 1;
        }
    }
    return true;
}

bool PLAMIOSpriteEditor::writeDataLine(std::string& line, void* arg)
{
    PLAMIOSpriteEditor* editor = static_cast<PLAMIOSpriteEditor*>(arg);
    if (!editor) return false;
    char buffer[96];
    if (editor->writeLineIndex == 0)
    {
        line.assign("SIZE=16x16");
        ++editor->writeLineIndex;
        return true;
    }

    const uint16_t row = editor->writeLineIndex - 1;
    if (row >= SPRITE_H) return false;
    int offset = snprintf(buffer, sizeof(buffer), "ROW=");
    for (uint8_t x = 0; x < SPRITE_W; ++x)
    {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%u%s",
                           static_cast<unsigned>(editor->pixels[row * SPRITE_W + x]),
                           (x + 1 < SPRITE_W) ? "," : "");
    }
    line.assign(buffer);
    ++editor->writeLineIndex;
    return true;
}

bool PLAMIOSpriteEditor::writeSourceLine(std::string& line, void* arg)
{
    PLAMIOSpriteEditor* editor = static_cast<PLAMIOSpriteEditor*>(arg);
    if (!editor) return false;
    char buffer[192];
    const uint16_t lineIndex = editor->writeLineIndex++;
    if (lineIndex == 0) { line.assign("#pragma once"); return true; }
    if (lineIndex == 1) { line.assign("#include \"PLAMIO.h\""); return true; }
    if (lineIndex == 2) { line.assign(""); return true; }
    if (lineIndex == 3)
    {
        snprintf(buffer, sizeof(buffer), "static const uint16_t slot%uSprite[16 * 16] =", static_cast<unsigned>(editor->writeSlot + 1));
        line.assign(buffer); return true;
    }
    if (lineIndex == 4) { line.assign("{"); return true; }
    if (lineIndex >= 5 && lineIndex < 21)
    {
        const uint8_t y = static_cast<uint8_t>(lineIndex - 5);
        int offset = 0;
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "    ");
        for (uint8_t x = 0; x < SPRITE_W; ++x)
        {
            const uint8_t index = editor->pixels[y * SPRITE_W + x];
            const uint16_t color = static_cast<uint16_t>(paletteColor(index));
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "0x%04X%s", color, (x == SPRITE_W - 1) ? "" : ", ");
        }
        if (y != SPRITE_H - 1) snprintf(buffer + offset, sizeof(buffer) - offset, ",");
        line.assign(buffer); return true;
    }
    if (lineIndex == 21) { line.assign("};"); return true; }
    if (lineIndex == 22) { line.assign(""); return true; }
    if (lineIndex == 23)
    {
        snprintf(buffer, sizeof(buffer), "// graphics.drawSprite(slot%uSprite, x, y, 16, 16, 1, PLAMIO::Graphics::BLACK, flipX, flipY);", static_cast<unsigned>(editor->writeSlot + 1));
        line.assign(buffer); return true;
    }
    return false;
}

void PLAMIOSpriteEditor::drawEditor(Graphics& graphics)
{
    graphics.fillScreen(COLOR_BG);
    graphics.drawString("SPRITE EDITOR", 10, 8, Graphics::WHITE, Graphics::SIZE_22B);
    graphics.drawString(mode == Mode::PREVIEW ? "PREVIEW" : "16x16", 310, 13, COLOR_ACCENT, Graphics::SIZE_18,
                        Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
    drawCanvas(graphics);
    drawPalette(graphics);

    graphics.drawString("ACTUAL", 247, 39, COLOR_TEXT_DIM, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.fillRect(PREVIEW_X - 3, PREVIEW_Y - 3, 38, 38, COLOR_PANEL);
    graphics.drawSprite(renderPixels, PREVIEW_X, PREVIEW_Y, SPRITE_W, SPRITE_H, 2,
                        paletteColor(0), false, false);

    graphics.drawString("A DRAW  B ERASE", 183, 112, Graphics::WHITE, Graphics::SIZE_13);
    graphics.drawString("X COLOR  Y PICK", 183, 130, Graphics::WHITE, Graphics::SIZE_13);
    graphics.drawString("START PREVIEW", 183, 148, COLOR_ACCENT, Graphics::SIZE_13);
    graphics.drawString("SELECT MENU", 183, 166, COLOR_ACCENT, Graphics::SIZE_13);
}

void PLAMIOSpriteEditor::drawCanvas(Graphics& graphics)
{
    graphics.fillRect(CANVAS_X, CANVAS_Y, CANVAS_SIZE, CANVAS_SIZE, COLOR_PANEL);
    for (uint8_t y = 0; y < SPRITE_H; ++y)
        for (uint8_t x = 0; x < SPRITE_W; ++x)
        {
            const int16_t px = CANVAS_X + x * CELL;
            const int16_t py = CANVAS_Y + y * CELL;
            graphics.fillRect(px + 1, py + 1, CELL - 1, CELL - 1,
                              paletteColor(pixels[y * SPRITE_W + x]));
        }
    for (uint8_t i = 0; i <= SPRITE_W; ++i)
        graphics.drawLine(CANVAS_X + i * CELL, CANVAS_Y,
                          CANVAS_X + i * CELL, CANVAS_Y + CANVAS_SIZE, COLOR_GRID);
    for (uint8_t i = 0; i <= SPRITE_H; ++i)
        graphics.drawLine(CANVAS_X, CANVAS_Y + i * CELL,
                          CANVAS_X + CANVAS_SIZE, CANVAS_Y + i * CELL, COLOR_GRID);
    if (mode == Mode::EDIT)
        graphics.drawRect(CANVAS_X + cursorX * CELL, CANVAS_Y + cursorY * CELL,
                          CELL + 1, CELL + 1, 2, COLOR_CURSOR);
}

void PLAMIOSpriteEditor::drawPalette(Graphics& graphics)
{
    const int16_t y = 207;
    for (uint8_t i = 0; i < PALETTE_SIZE; ++i)
    {
        const int16_t x = 10 + i * 19;
        graphics.fillRect(x, y, 15, 15, paletteColor(i));
        graphics.drawRect(x, y, 15, 15, (i == selectedColor) ? 2 : 1,
                          (i == selectedColor) ? COLOR_CURSOR : COLOR_GRID);
    }
}

void PLAMIOSpriteEditor::drawPreview(Graphics& graphics)
{
    graphics.fillRect(176, 34, 138, 167, COLOR_PANEL);
    graphics.drawString("PREVIEW", 245, 43, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawSprite(renderPixels, 213, 77, SPRITE_W, SPRITE_H, 4, paletteColor(0), false, false);
    graphics.drawString("NORMAL", 245, 147, COLOR_TEXT_DIM, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString("START: BACK", 245, 175, COLOR_ACCENT, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
}

void PLAMIOSpriteEditor::drawMainMenu(Graphics& graphics)
{
    graphics.fillRoundRect(72, 34, 176, 169, 8, COLOR_PANEL);
    graphics.drawRoundRect(72, 34, 176, 169, 8, 3, COLOR_CURSOR);
    graphics.drawString("SPRITE MENU", 160, 47, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < MAIN_MENU_COUNT; ++i)
    {
        const int16_t y = 78 + i * 19;
        if (i == menuIndex) graphics.fillRoundRect(88, y - 2, 144, 18, 4, COLOR_ACCENT);
        graphics.drawString(MAIN_MENU_ITEMS[i], 160, y,
                            (i == menuIndex) ? COLOR_BG : Graphics::WHITE,
                            Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER,
                            Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOSpriteEditor::drawSlotMenu(Graphics& graphics, bool saving)
{
    graphics.fillRoundRect(72, 48, 176, 132, 8, COLOR_PANEL);
    graphics.drawRoundRect(72, 48, 176, 132, 8, 3, COLOR_CURSOR);
    graphics.drawString(saving ? "SAVE SPRITE" : "LOAD SPRITE", 160, 61,
                        Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < SLOT_COUNT; ++i)
    {
        char text[32];
        snprintf(text, sizeof(text), "SLOT %u   %s", static_cast<unsigned>(i + 1), slotExists[i] ? "DATA" : "EMPTY");
        const int16_t y = 99 + i * 23;
        if (i == menuIndex) graphics.fillRoundRect(78, y - 2, 164, 20, 4, COLOR_ACCENT);
        graphics.drawString(text, 160, y, (i == menuIndex) ? COLOR_BG : Graphics::WHITE,
                            Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER,
                            Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOSpriteEditor::drawTransformMenu(Graphics& graphics)
{
    graphics.fillRoundRect(72, 53, 176, 121, 8, COLOR_PANEL);
    graphics.drawRoundRect(72, 53, 176, 121, 8, 3, COLOR_CURSOR);
    graphics.drawString("TRANSFORM", 160, 66, Graphics::WHITE, Graphics::SIZE_22B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    for (uint8_t i = 0; i < TRANSFORM_COUNT; ++i)
    {
        const int16_t y = 103 + i * 24;
        if (i == menuIndex) graphics.fillRoundRect(91, y - 2, 138, 20, 4, COLOR_ACCENT);
        graphics.drawString(TRANSFORM_ITEMS[i], 160, y,
                            (i == menuIndex) ? COLOR_BG : Graphics::WHITE,
                            Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER,
                            Graphics::VerticalAlign::TOP);
    }
}

void PLAMIOSpriteEditor::drawConfirmation(Graphics& graphics, const char* title,
                                    const char* message, const char* yesText)
{
    graphics.fillRoundRect(40, 66, 240, 104, 8, COLOR_PANEL);
    graphics.drawRoundRect(40, 66, 240, 104, 8, 3, COLOR_CURSOR);
    graphics.drawString(title, 160, 79, Graphics::WHITE, Graphics::SIZE_25B,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString(message, 160, 113, COLOR_TEXT_DIM, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString(yesText, 95, 143, COLOR_ACCENT, Graphics::SIZE_18,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
    graphics.drawString("B: CANCEL", 222, 143, Graphics::WHITE, Graphics::SIZE_18,
                        Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::TOP);
}

Graphics::Color PLAMIOSpriteEditor::paletteColor(uint8_t index)
{
    static constexpr Graphics::Color palette[PALETTE_SIZE] = {
        Graphics::BLACK,
        Graphics::WHITE,
        Graphics::rgb565(74, 84, 96),
        Graphics::rgb565(165, 175, 185),
        Graphics::rgb565(225, 65, 75),
        Graphics::rgb565(245, 135, 55),
        Graphics::rgb565(250, 215, 70),
        Graphics::rgb565(75, 195, 95),
        Graphics::rgb565(45, 150, 125),
        Graphics::rgb565(60, 185, 210),
        Graphics::rgb565(70, 110, 225),
        Graphics::rgb565(105, 75, 185),
        Graphics::rgb565(205, 75, 180),
        Graphics::rgb565(235, 125, 175),
        Graphics::rgb565(120, 75, 45),
        Graphics::rgb565(255, 235, 200)
    };
    return palette[index < PALETTE_SIZE ? index : 0];
}
