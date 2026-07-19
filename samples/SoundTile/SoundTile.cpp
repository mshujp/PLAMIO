#include "SoundTile.h"
#include <cstdio>

using namespace PLAMIO;

namespace
{
constexpr int16_t SCREEN_W = 320;
constexpr int16_t SCREEN_H = 240;
constexpr int16_t GRID_X = 8;
constexpr int16_t GRID_Y = 18;
constexpr int16_t TILE_W = 56;
constexpr int16_t TILE_H = 45;
constexpr int16_t TILE_GAP = 6;
constexpr uint32_t FLASH_MSEC = 150;

static const char* TILE_LABELS[20] = {
    "CHIME", "PLUCK", "BLOOM", "DROP", "TAP",
    "BELL", "POPPY", "WARM", "CLICK", "BUBBLE",
    "RING", "SOFT", "WOOD", "SPARK", "MELLOW",
    "TONE", "GLASS", "ROUND", "PING", "SUNNY"
};

static const Audio::SoundStep SE_01_STEPS[] = {
    { 620, 880, 45, 0.75f, 0.95f },
    { 880, 1240, 70, 0.95f, 0.00f }
};

static const Audio::SoundStep SE_02_STEPS[] = {
    { 980, 760, 35, 0.90f, 0.75f },
    { 760, 520, 55, 0.75f, 0.00f }
};

static const Audio::SoundStep SE_03_STEPS[] = {
    { 360, 520, 60, 0.55f, 0.80f },
    { 520, 760, 80, 0.80f, 0.00f }
};

static const Audio::SoundStep SE_04_STEPS[] = {
    { 860, 420, 90, 0.85f, 0.00f }
};

static const Audio::SoundStep SE_05_STEPS[] = {
    { 1250, 1020, 24, 0.85f, 0.00f },
    { 0, 0, 18, 0.00f, 0.00f },
    { 980, 820, 28, 0.70f, 0.00f }
};

static const Audio::SoundStep SE_06_STEPS[] = {
    { 740, 740, 60, 0.70f, 0.50f },
    { 1110, 1110, 100, 0.50f, 0.00f }
};

static const Audio::SoundStep SE_07_STEPS[] = {
    { 1450, 1750, 32, 0.80f, 0.00f },
    { 0, 0, 20, 0.00f, 0.00f },
    { 1650, 2050, 36, 0.70f, 0.00f }
};

static const Audio::SoundStep SE_08_STEPS[] = {
    { 280, 380, 70, 0.55f, 0.75f },
    { 380, 500, 90, 0.75f, 0.00f }
};

static const Audio::SoundStep SE_09_STEPS[] = {
    { 1800, 1350, 18, 0.75f, 0.00f }
};

static const Audio::SoundStep SE_10_STEPS[] = {
    { 420, 610, 45, 0.65f, 0.80f },
    { 610, 390, 55, 0.80f, 0.00f }
};

static const Audio::SoundStep SE_11_STEPS[] = {
    { 520, 1040, 90, 0.60f, 0.85f },
    { 1040, 780, 80, 0.85f, 0.00f }
};

static const Audio::SoundStep SE_12_STEPS[] = {
    { 680, 610, 85, 0.45f, 0.00f }
};

static const Audio::SoundStep SE_13_STEPS[] = {
    { 210, 170, 38, 0.80f, 0.00f },
    { 0, 0, 12, 0.00f, 0.00f },
    { 260, 210, 42, 0.65f, 0.00f }
};

static const Audio::SoundStep SE_14_STEPS[] = {
    { 1180, 1580, 40, 0.65f, 0.90f },
    { 1580, 2280, 45, 0.90f, 0.00f }
};

static const Audio::SoundStep SE_15_STEPS[] = {
    { 470, 620, 65, 0.50f, 0.65f },
    { 620, 540, 95, 0.65f, 0.00f }
};

static const Audio::SoundStep SE_16_STEPS[] = {
    { 330, 330, 45, 0.65f, 0.55f },
    { 440, 440, 55, 0.55f, 0.45f },
    { 550, 550, 80, 0.45f, 0.00f }
};

static const Audio::SoundStep SE_17_STEPS[] = {
    { 1320, 1320, 40, 0.65f, 0.45f },
    { 1980, 1980, 80, 0.45f, 0.00f }
};

static const Audio::SoundStep SE_18_STEPS[] = {
    { 520, 450, 45, 0.75f, 0.50f },
    { 450, 360, 75, 0.50f, 0.00f }
};

static const Audio::SoundStep SE_19_STEPS[] = {
    { 900, 1250, 28, 0.80f, 0.00f },
    { 0, 0, 16, 0.00f, 0.00f },
    { 1250, 1500, 32, 0.65f, 0.00f }
};

static const Audio::SoundStep SE_20_STEPS[] = {
    { 540, 720, 45, 0.60f, 0.80f },
    { 720, 960, 45, 0.80f, 0.90f },
    { 960, 1280, 70, 0.90f, 0.00f }
};

static const Audio::Sound CUSTOM_SE[20] = {
    { SE_01_STEPS, 2 }, { SE_02_STEPS, 2 },
    { SE_03_STEPS, 2 }, { SE_04_STEPS, 1 },
    { SE_05_STEPS, 3 }, { SE_06_STEPS, 2 },
    { SE_07_STEPS, 3 }, { SE_08_STEPS, 2 },
    { SE_09_STEPS, 1 }, { SE_10_STEPS, 2 },
    { SE_11_STEPS, 2 }, { SE_12_STEPS, 1 },
    { SE_13_STEPS, 3 }, { SE_14_STEPS, 2 },
    { SE_15_STEPS, 2 }, { SE_16_STEPS, 3 },
    { SE_17_STEPS, 2 }, { SE_18_STEPS, 2 },
    { SE_19_STEPS, 3 }, { SE_20_STEPS, 3 }
};
}

const char* SoundTile::getId() const
{
    return "soundtiles";
}

const char* SoundTile::getName() const
{
    return "Sound Tiles";
}

const char* SoundTile::getMenuName() const
{
    return "03 Sound Tiles";
}

uint16_t SoundTile::getLogicalScreenWidth() const
{
    return 320;
}

uint16_t SoundTile::getLogicalScreenHeight() const
{
    return 240;
}

uint16_t SoundTile::getTargetScreenWidth() const
{
    return 320;
}

uint16_t SoundTile::getTargetScreenHeight() const
{
    return 240;
}

void SoundTile::onInit(Storage& storage)
{
    (void)storage;
    selectedTile = 0;
    lastPlayedTile = 255;
    flashUntilMsec = 0;
    dirty = true;
}

Game::GameState SoundTile::onUpdate(
    Input& input,
    Audio& audio,
    Storage& storage,
    float lastSec
)
{
    (void)storage;

    if (input.justPressed(Input::LEFT)) {
        moveSelection(-1, 0);
    }
    if (input.justPressed(Input::RIGHT)) {
        moveSelection(1, 0);
    }
    if (input.justPressed(Input::UP)) {
        moveSelection(0, -1);
    }
    if (input.justPressed(Input::DOWN)) {
        moveSelection(0, 1);
    }

    if (input.justPressed(Input::A)) {
        audio.playSE(&CUSTOM_SE[selectedTile], 1.0f);
        lastPlayedTile = selectedTile;
        flashUntilMsec = Platform::getMsec() + FLASH_MSEC;
        dirty = true;
    }

    if (lastPlayedTile != 255 &&
        Platform::getMsec() >= flashUntilMsec) {
        lastPlayedTile = 255;
        dirty = true;
    }

    return GameState::RUNNING;
}

bool SoundTile::onDraw(
    Graphics& graphics,
    bool requestFullRedraw
)
{
    if (!requestFullRedraw && !dirty) {
        return false;
    }

    const Graphics::Color bg =
        Graphics::rgb565(247, 232, 202);
    const Graphics::Color panel =
        Graphics::rgb565(236, 205, 158);
    const Graphics::Color tile =
        Graphics::rgb565(255, 244, 218);
    const Graphics::Color tileAlt =
        Graphics::rgb565(248, 224, 184);
    const Graphics::Color border =
        Graphics::rgb565(173, 111, 72);
    const Graphics::Color selected =
        Graphics::rgb565(225, 142, 83);
    const Graphics::Color flash =
        Graphics::rgb565(244, 187, 92);
    const Graphics::Color text =
        Graphics::rgb565(92, 58, 38);
    const Graphics::Color softText =
        Graphics::rgb565(132, 86, 54);

    graphics.fillScreen(panel);

    for (uint8_t i = 0; i < TILE_COUNT; ++i) {
        const uint8_t col = i % COLS;
        const uint8_t row = i / COLS;

        const int16_t x =
            GRID_X + col * (TILE_W + TILE_GAP);
        const int16_t y =
            GRID_Y + row * (TILE_H + TILE_GAP);

        Graphics::Color fill =
            ((row + col) & 1) ? tileAlt : tile;

        if (i == lastPlayedTile) {
            fill = flash;
        } else if (i == selectedTile) {
            fill = selected;
        }

        graphics.fillRoundRect(
            x, y, TILE_W, TILE_H, 7, fill
        );
        graphics.drawRoundRect(
            x, y, TILE_W, TILE_H, 7, 2, border
        );

        char number[4];
        snprintf(number, sizeof(number), "%02u", i + 1);

        graphics.drawString(
            number,
            x + TILE_W / 2,
            y + 7,
            text,
            Graphics::SIZE_18,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::TOP
        );

        graphics.drawString(
            TILE_LABELS[i],
            x + TILE_W / 2,
            y + 28,
            softText,
            Graphics::SIZE_10,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::MIDDLE
        );
    }

    graphics.drawString(
        "D-PAD SELECT   A PLAY",
        160,
        222,
        text,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP
    );

    dirty = false;
    return true;
}

void SoundTile::onTerminate(Storage& storage)
{
    (void)storage;
}

void SoundTile::moveSelection(int8_t dx, int8_t dy)
{
    int8_t col = selectedTile % COLS;
    int8_t row = selectedTile / COLS;

    col = (col + dx + COLS) % COLS;
    row = (row + dy + ROWS) % ROWS;

    selectedTile =
        static_cast<uint8_t>(row * COLS + col);
    dirty = true;
}
