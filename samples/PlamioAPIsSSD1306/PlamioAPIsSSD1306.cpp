#include "PlamioAPIsSSD1306.h"

#include <cmath>
#include <cstdio>

using PLAMIO::Audio;
using PLAMIO::Graphics;
using PLAMIO::Input;
namespace Platform = PLAMIO::Platform;
using PLAMIO::Storage;

namespace {
constexpr uint16_t TARGET_W = PLAMIO::Display::SSD1306_SCREEN_W;
constexpr uint16_t TARGET_H = PLAMIO::Display::SSD1306_SCREEN_H;
constexpr uint16_t LOGICAL_W = TARGET_W; 
constexpr uint16_t LOGICAL_H = TARGET_H;

constexpr uint32_t STEP_MSEC = 3500;
constexpr uint32_t HOLD_MSEC = 1400;
constexpr uint8_t STEP_COUNT = 20;

constexpr Graphics::Color OFF = Graphics::SSD1306_OFF;
constexpr Graphics::Color ON = Graphics::SSD1306_ON;

const char* const STEP_NAMES[STEP_COUNT] = {
    "clearScreen",
    "fillScreen",
    "drawPixel",
    "drawLine",
    "drawTriangle",
    "fillTriangle",
    "drawRect",
    "drawRoundRect",
    "drawRect thick",
    "drawRound thick",
    "fillRect",
    "fillRoundRect",
    "drawCircle",
    "drawCircle XY",
    "fillCircle",
    "fillCircle XY",
    "Font / measure",
    "Alignment",
    "Viewport",
    "Sprite"
};

// 8x8 monochrome-friendly speaker-like sprite.
// BLACK is transparent when drawn.
static const uint16_t TEST_SPRITE[8 * 8] = {
    OFF,OFF,ON, ON, OFF,OFF,OFF,OFF,
    OFF,ON, ON, ON, OFF,ON, OFF,OFF,
    ON, ON, ON, ON, OFF,OFF,ON, OFF,
    ON, ON, ON, ON, OFF,ON, ON, OFF,
    ON, ON, ON, ON, OFF,ON, ON, OFF,
    ON, ON, ON, ON, OFF,OFF,ON, OFF,
    OFF,ON, ON, ON, OFF,ON, OFF,OFF,
    OFF,OFF,ON, ON, OFF,OFF,OFF,OFF
};
}

const char* PlamioAPIsSSD1306::getId() const { return "plamioapis1306sample"; }
const char* PlamioAPIsSSD1306::getName() const { return "PLAMIO APIs SSD1306"; }
const char* PlamioAPIsSSD1306::getMenuName() const { return "01 PLAMIO APIs for SSD1306"; }

uint16_t PlamioAPIsSSD1306::getLogicalScreenWidth() const { return LOGICAL_W; }
uint16_t PlamioAPIsSSD1306::getLogicalScreenHeight() const { return LOGICAL_H; }
uint16_t PlamioAPIsSSD1306::getTargetScreenWidth() const { return TARGET_W; }
uint16_t PlamioAPIsSSD1306::getTargetScreenHeight() const { return TARGET_H; }

void PlamioAPIsSSD1306::onInit(Storage& storage) {
    (void)storage;
    mode = Mode::TITLE;
    drawStep = 0;
    inputMask = 0;
    stepStartMsec = Platform::getMsec();
    speakerIconUntilMsec = 0;
    dirty = true;
}

PLAMIO::Game::GameState PlamioAPIsSSD1306::onUpdate(
    Input& input, Audio& audio, Storage& storage, float deltaSec) {
    (void)storage;
    (void)deltaSec;

    updateInputMask(input);

    switch (mode) {
        case Mode::TITLE:
            updateTitle(input, audio);
            break;
        case Mode::GRAPHICS:
            updateGraphics(input, audio);
            break;
    }

    // The speaker icon must disappear even when no button is pressed.
    if (speakerIconUntilMsec != 0 &&
        static_cast<int32_t>(Platform::getMsec() - speakerIconUntilMsec) >= 0) {
        speakerIconUntilMsec = 0;
        dirty = true;
    }

    return GameState::RUNNING;
}

bool PlamioAPIsSSD1306::onDraw(Graphics& graphics, bool requestFullRedraw) {
    if (!requestFullRedraw && !dirty) {
        return false;
    }

    graphics.resetViewport();

    switch (mode) {
        case Mode::TITLE:
            drawTitle(graphics);
            break;
        case Mode::GRAPHICS:
            drawGraphicsTest(graphics);
            break;
    }

    dirty = false;
    return true;
}

void PlamioAPIsSSD1306::onTerminate(Storage& storage) {
    (void)storage;
}

void PlamioAPIsSSD1306::enterGraphics(Audio& audio) {
    mode = Mode::GRAPHICS;
    drawStep = 0;
    stepStartMsec = Platform::getMsec();
    playTestSE(audio, &Audio::SE::NO_8, 0.55f, 300);
    dirty = true;
}

void PlamioAPIsSSD1306::changeStep(int8_t amount, Audio& audio) {
    int16_t next = static_cast<int16_t>(drawStep) + amount;
    while (next < 0) next += STEP_COUNT;
    while (next >= STEP_COUNT) next -= STEP_COUNT;

    drawStep = static_cast<uint8_t>(next);
    stepStartMsec = Platform::getMsec();

    const Audio::Sound* sound = amount >= 0 ? &Audio::SE::NO_1 : &Audio::SE::NO_2;
    playTestSE(audio, sound, 0.40f, 180);
    dirty = true;
}

void PlamioAPIsSSD1306::playTestSE(
    Audio& audio, const Audio::Sound* sound, float gain, uint16_t iconMsec) {
    audio.playSE(sound, gain);
    speakerIconUntilMsec = Platform::getMsec() + iconMsec;
    dirty = true;
}

void PlamioAPIsSSD1306::updateInputMask(Input& input) {
    const uint16_t previous = inputMask;
    inputMask = 0;

    if (input.pressed(Input::UP)) inputMask |= Input::UP;
    if (input.pressed(Input::DOWN)) inputMask |= Input::DOWN;
    if (input.pressed(Input::LEFT)) inputMask |= Input::LEFT;
    if (input.pressed(Input::RIGHT)) inputMask |= Input::RIGHT;
    if (input.pressed(Input::A)) inputMask |= Input::A;
    if (input.pressed(Input::B)) inputMask |= Input::B;

    if (inputMask != previous) {
        dirty = true;
    }
}

void PlamioAPIsSSD1306::updateTitle(Input& input, Audio& audio) {
    if (input.justPressed(Input::A)) {
        enterGraphics(audio);
        return;
    }

    if (input.justPressed(Input::UP) ||
        input.justPressed(Input::DOWN) ||
        input.justPressed(Input::LEFT) ||
        input.justPressed(Input::RIGHT) ||
        input.justPressed(Input::B)) {
        playTestSE(audio, &Audio::SE::NO_1, 0.30f, 150);
    }
}

void PlamioAPIsSSD1306::updateGraphics(Input& input, Audio& audio) {
    // Moving graphics and viewport animation require continuous redraw.
    dirty = true;

    if (input.justPressed(Input::A)) {
        changeStep(1, audio);
        return;
    }
    if (input.justPressed(Input::B)) {
        changeStep(-1, audio);
        return;
    }

    if (input.justPressed(Input::UP) ||
        input.justPressed(Input::DOWN) ||
        input.justPressed(Input::LEFT) ||
        input.justPressed(Input::RIGHT)) {
        playTestSE(audio, &Audio::SE::NO_1, 0.25f, 130);
    }

    if (Platform::getMsec() - stepStartMsec >= STEP_MSEC) {
        changeStep(1, audio);
    }
}

void PlamioAPIsSSD1306::drawTitle(Graphics& g) {
    g.clearScreen();
    g.drawRoundRect(1, 1, 126, 62, 5, ON);
    g.drawString("PLAMIO", 64, 9, ON, Graphics::SIZE_18,
                 Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::TOP);
    g.drawString("API TEST", 64, 29, ON, Graphics::SIZE_13,
                 Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::TOP);
    g.drawString("SSD1306", 64, 43, ON, Graphics::SIZE_10,
                 Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::TOP);
    g.drawString("A:START", 64, 61, ON, Graphics::SIZE_10,
                 Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::BOTTOM);

    drawInputOverlay(g);
}

void PlamioAPIsSSD1306::drawGraphicsTest(Graphics& g) {
    if (drawStep == 18) {
        drawViewportTest(g);
    } else {
        g.resetViewport();
        g.clearScreen();
        const int16_t x = getAnimX(Platform::getMsec());
        drawMovingShape(g, drawStep, x, 36);
    }

    // Keep UI fixed even while the viewport test moves the logical image.
    g.fillRect(0, 0, TARGET_W, 11, OFF);
    g.drawLine(0, 12, TARGET_W - 1, 12, ON);

    char header[32];
    snprintf(header, sizeof(header), "%02u/%02u %s",
             static_cast<unsigned>(drawStep + 1),
             static_cast<unsigned>(STEP_COUNT),
             getStepName());
    g.drawString(header, 2, 1, ON, Graphics::SIZE_10);

    drawInputOverlay(g);
}

void PlamioAPIsSSD1306::drawMovingShape(
    Graphics& g, uint8_t step, int16_t x, int16_t y) {
    switch (step) {
        case 0:
            g.fillScreen(ON);
            g.clearScreen();
            g.drawCircle(x, y, 12, ON);
            break;
        case 1:
            g.fillScreen(ON);
            g.fillRect(x - 10, y - 10, 20, 20, OFF);
            break;
        case 2:
            for (int16_t i = 0; i < 30; ++i) {
                g.drawPixel(x + ((i * 11) % 35) - 17,
                            y + ((i * 7) % 25) - 12,
                            ON);
            }
            break;
        case 3:
            g.drawLine(x - 20, y - 13, x + 20, y + 13, ON);
            g.drawLine(x - 20, y + 13, x + 20, y - 13, ON);
            break;
        case 4:
            g.drawTriangle(x, y - 16, x - 19, y + 14, x + 19, y + 14, ON);
            break;
        case 5:
            g.fillTriangle(x, y - 16, x - 19, y + 14, x + 19, y + 14, ON);
            break;
        case 6:
            g.drawRect(x - 22, y - 14, 44, 28, ON);
            break;
        case 7:
            g.drawRoundRect(x - 22, y - 14, 44, 28, 6, ON);
            break;
        case 8:
            g.drawRect(x - 22, y - 14, 44, 28, 3, ON);
            break;
        case 9:
            g.drawRoundRect(x - 22, y - 14, 44, 28, 6, 3, ON);
            break;
        case 10:
            g.fillRect(x - 22, y - 14, 44, 28, ON);
            break;
        case 11:
            g.fillRoundRect(x - 22, y - 14, 44, 28, 6, ON);
            break;
        case 12:
            g.drawCircle(x, y, 15, ON);
            break;
        case 13:
            g.drawCircle(x, y, 22, 11, ON);
            break;
        case 14:
            g.fillCircle(x, y, 15, ON);
            break;
        case 15:
            g.fillCircle(x, y, 22, 11, ON);
            break;
        case 16:
            drawFontTest(g);
            break;
        case 17:
            drawAlignmentTest(g);
            break;
        case 19:
            drawSpriteTest(g);
            break;
        default:
            break;
    }
}

void PlamioAPIsSSD1306::drawFontTest(Graphics& g) {
    g.drawString("SIZE_10", 3, 14, ON, Graphics::SIZE_10);
    g.drawString("SIZE_13", 3, 25, ON, Graphics::SIZE_13);
    g.drawString("SIZE_18", 3, 39, ON, Graphics::SIZE_18);

    const uint16_t width = g.getTextWidth("WIDTH", Graphics::SIZE_10);
    const uint16_t height = g.getTextHeight("WIDTH", Graphics::SIZE_10);
    char text[20];
    snprintf(text, sizeof(text), "W%u H%u",
             static_cast<unsigned>(width),
             static_cast<unsigned>(height));
    g.drawString(text, 126, 63, ON, Graphics::SIZE_10,
                 Graphics::HorizontalAlign::RIGHT,
                 Graphics::VerticalAlign::BOTTOM);
}

void PlamioAPIsSSD1306::drawAlignmentTest(Graphics& g) {
    const int16_t xs[3] = {20, 64, 108};
    const int16_t ys[3] = {19, 36, 54};
    const Graphics::HorizontalAlign ha[3] = {
        Graphics::HorizontalAlign::LEFT,
        Graphics::HorizontalAlign::CENTER,
        Graphics::HorizontalAlign::RIGHT
    };
    const Graphics::VerticalAlign va[3] = {
        Graphics::VerticalAlign::TOP,
        Graphics::VerticalAlign::MIDDLE,
        Graphics::VerticalAlign::BOTTOM
    };
    const char* const labels[3][3] = {
        {"LT", "CT", "RT"},
        {"LM", "CM", "RM"},
        {"LB", "CB", "RB"}
    };

    for (uint8_t yi = 0; yi < 3; ++yi) {
        for (uint8_t xi = 0; xi < 3; ++xi) {
            g.drawPixel(xs[xi], ys[yi], ON);
            g.drawString(labels[yi][xi], xs[xi], ys[yi], ON,
                         Graphics::SIZE_10, ha[xi], va[yi]);
        }
    }
}

void PlamioAPIsSSD1306::drawViewportTest(Graphics& g) {
    const uint32_t elapsed = Platform::getMsec() - stepStartMsec;
    const float angle = static_cast<float>(elapsed) * 0.004f;
    const int16_t vx = static_cast<int16_t>(std::sin(angle) * 24.0f);
    const int16_t vy = static_cast<int16_t>(std::cos(angle) * 12.0f);

    g.setViewport(vx, vy);
    g.clearScreen();

    for (int16_t x = -64; x < static_cast<int16_t>(LOGICAL_W); x += 16) {
        g.drawLine(x, -32, x, LOGICAL_H - 1, ON);
    }
    for (int16_t y = -32; y < static_cast<int16_t>(LOGICAL_H); y += 16) {
        g.drawLine(-64, y, LOGICAL_W - 1, y, ON);
    }

    g.fillCircle(64, 32, 9, ON);
    g.drawRoundRect(37, 15, 54, 34, 5, 2, ON);
}

void PlamioAPIsSSD1306::drawSpriteTest(Graphics& g) {
    const int16_t x = getAnimX(Platform::getMsec());
    g.drawSprite(TEST_SPRITE, x - 8, 20, 8, 8, 1, OFF);
    g.drawSprite(TEST_SPRITE, x, 27, 8, 8, 2, OFF);
    g.drawSprite(TEST_SPRITE, x + 18, 35, 8, 8, 3, OFF, true, false);
}

void PlamioAPIsSSD1306::drawInputOverlay(Graphics& g) {
    if ((inputMask & Input::UP) != 0) {
        g.drawLine(0, 0, TARGET_W - 1, 0, ON);
    }
    if ((inputMask & Input::DOWN) != 0) {
        g.drawLine(0, TARGET_H - 1, TARGET_W - 1, TARGET_H - 1, ON);
    }
    if ((inputMask & Input::LEFT) != 0) {
        g.drawLine(0, 0, 0, TARGET_H - 1, ON);
    }
    if ((inputMask & Input::RIGHT) != 0) {
        g.drawLine(TARGET_W - 1, 0, TARGET_W - 1, TARGET_H - 1, ON);
    }

    // A/B input lamps. Outline is always visible; fill means pressed.
    if ((inputMask & Input::A) != 0) g.fillCircle(121, 21, 4, ON);
    else g.drawCircle(121, 21, 4, ON);

    if ((inputMask & Input::B) != 0) g.fillCircle(121, 48, 4, ON);
    else g.drawCircle(121, 48, 4, ON);

    g.drawString("A", 121, 21, (inputMask & Input::A) != 0 ? OFF : ON,
                 Graphics::SIZE_10,
                 Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::MIDDLE);
    g.drawString("B", 121, 48, (inputMask & Input::B) != 0 ? OFF : ON,
                 Graphics::SIZE_10,
                 Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::MIDDLE);

    if (speakerIconUntilMsec != 0) {
        drawSpeakerIcon(g, 108, 1);
    }
}

void PlamioAPIsSSD1306::drawSpeakerIcon(Graphics& g, int16_t x, int16_t y) {
    g.fillRect(x, y + 3, 3, 4, ON);
    g.fillTriangle(x + 2, y + 3, x + 6, y, x + 6, y + 9, ON);
    g.drawLine(x + 8, y + 2, x + 9, y + 3, ON);
    g.drawLine(x + 9, y + 3, x + 9, y + 6, ON);
    g.drawLine(x + 9, y + 6, x + 8, y + 7, ON);
}

int16_t PlamioAPIsSSD1306::getAnimX(uint32_t now) const {
    const uint32_t elapsed = now - stepStartMsec;
    const uint32_t travelMsec = (STEP_MSEC - HOLD_MSEC) / 2;

    if (elapsed < travelMsec) {
        return static_cast<int16_t>(150 -
            (86L * static_cast<int32_t>(elapsed)) /
            static_cast<int32_t>(travelMsec));
    }

    if (elapsed < travelMsec + HOLD_MSEC) {
        return 64;
    }

    uint32_t outElapsed = elapsed - travelMsec - HOLD_MSEC;
    if (outElapsed > travelMsec) outElapsed = travelMsec;

    return static_cast<int16_t>(64 -
        (96L * static_cast<int32_t>(outElapsed)) /
        static_cast<int32_t>(travelMsec));
}

const char* PlamioAPIsSSD1306::getStepName() const {
    return drawStep < STEP_COUNT ? STEP_NAMES[drawStep] : "Graphics";
}
