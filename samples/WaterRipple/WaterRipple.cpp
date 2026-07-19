#include "WaterRipple.h"

using namespace PLAMIO;

namespace
{
constexpr int16_t SCREEN_W = 320;
constexpr int16_t SCREEN_H = 240;
constexpr float CENTER_X = 160.0f;
constexpr float CENTER_Y = 116.0f;
constexpr uint32_t AUTO_INTERVAL_MSEC = 650;

const Graphics::Color COLOR_BG = Graphics::rgb565(5, 24, 36);
const Graphics::Color COLOR_WATER = Graphics::rgb565(8, 49, 67);
const Graphics::Color COLOR_GRID = Graphics::rgb565(15, 68, 84);
const Graphics::Color COLOR_RIPPLE = Graphics::rgb565(118, 221, 232);
const Graphics::Color COLOR_RIPPLE_DIM = Graphics::rgb565(45, 135, 153);
const Graphics::Color COLOR_TEXT = Graphics::rgb565(214, 242, 242);
const Graphics::Color COLOR_ACCENT = Graphics::rgb565(255, 209, 113);
}

void WaterRipple::onInit(Storage& storage)
{
    (void)storage;
    clearRipples();
    autoEmit = true;
    lastAutoEmitMsec = Platform::getMsec();
    dirty = true;
}

Game::GameState WaterRipple::onUpdate(Input& input,
                                      Audio& audio,
                                      Storage& storage,
                                      float deltaSec)
{
    (void)storage;

    const uint32_t now = Platform::getMsec();

    if (input.justPressed(Input::A))
    {
        spawnRipple(CENTER_X, CENTER_Y, 58.0f, 2.2f);
        audio.playSE(&Audio::SE::NO_6, 0.45f);
    }

    if (input.justPressed(Input::X))
    {
        autoEmit = !autoEmit;
        lastAutoEmitMsec = now;
        audio.playSE(&Audio::SE::NO_1, 0.55f);
        dirty = true;
    }

    if (input.justPressed(Input::B))
    {
        clearRipples();
        audio.playSE(&Audio::SE::NO_2, 0.45f);
    }

    if (autoEmit && Platform::elapsed(now, lastAutoEmitMsec, AUTO_INTERVAL_MSEC))
    {
        lastAutoEmitMsec = now;
        spawnRipple(randomRange(54.0f, 266.0f),
                    randomRange(58.0f, 176.0f),
                    randomRange(46.0f, 66.0f),
                    randomRange(1.8f, 2.7f));
    }

    updateRipples(deltaSec);
    return Game::RUNNING;
}

bool WaterRipple::onDraw(Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty)
    {
        return false;
    }

    graphics.fillScreen(COLOR_BG);
    graphics.fillRoundRect(8, 30, 304, 172, 12, COLOR_WATER);

    for (int16_t y = 46; y < 194; y += 18)
    {
        graphics.drawLine(18, y, 302, y, COLOR_GRID);
    }

    graphics.drawString("WATER RIPPLE", 160, 8,
                        COLOR_TEXT, Graphics::SIZE_18,
                        Graphics::HorizontalAlign::CENTER,
                        Graphics::VerticalAlign::TOP);

    for (uint8_t i = 0; i < MAX_RIPPLES; ++i)
    {
        const Ripple& ripple = ripples[i];
        if (!ripple.active)
        {
            continue;
        }

        const float t = ripple.life / ripple.maxLife;
        const int16_t x = static_cast<int16_t>(ripple.x);
        const int16_t y = static_cast<int16_t>(ripple.y);
        const int16_t radius = static_cast<int16_t>(ripple.radius);

        graphics.drawCircle(x, y, radius,
                            t > 0.45f ? COLOR_RIPPLE : COLOR_RIPPLE_DIM);

        if (radius > 8)
        {
            graphics.drawCircle(x, y, radius - 5, COLOR_RIPPLE_DIM);
        }
    }

    graphics.fillCircle(static_cast<int16_t>(CENTER_X),
                        static_cast<int16_t>(CENTER_Y),
                        2, COLOR_ACCENT);

    graphics.drawString(autoEmit ? "AUTO ON" : "AUTO OFF", 16, 208,
                        autoEmit ? COLOR_ACCENT : COLOR_TEXT,
                        Graphics::SIZE_13,
                        Graphics::HorizontalAlign::LEFT,
                        Graphics::VerticalAlign::TOP);

    graphics.drawString("A RIPPLE   X AUTO   B CLEAR", 304, 208,
                        COLOR_TEXT, Graphics::SIZE_13,
                        Graphics::HorizontalAlign::RIGHT,
                        Graphics::VerticalAlign::TOP);

    dirty = false;
    return true;
}

void WaterRipple::onTerminate(Storage& storage)
{
    (void)storage;
}

void WaterRipple::clearRipples()
{
    for (uint8_t i = 0; i < MAX_RIPPLES; ++i)
    {
        ripples[i].active = false;
    }
    dirty = true;
}

void WaterRipple::spawnRipple(float x, float y, float speed, float lifetime)
{
    for (uint8_t i = 0; i < MAX_RIPPLES; ++i)
    {
        Ripple& ripple = ripples[i];
        if (ripple.active)
        {
            continue;
        }

        ripple.x = x;
        ripple.y = y;
        ripple.radius = 2.0f;
        ripple.speed = speed;
        ripple.life = lifetime;
        ripple.maxLife = lifetime;
        ripple.active = true;
        dirty = true;
        return;
    }
}

void WaterRipple::updateRipples(float deltaSec)
{
    bool anyActive = false;

    for (uint8_t i = 0; i < MAX_RIPPLES; ++i)
    {
        Ripple& ripple = ripples[i];
        if (!ripple.active)
        {
            continue;
        }

        ripple.radius += ripple.speed * deltaSec;
        ripple.speed *= (1.0f - 0.42f * deltaSec);
        ripple.life -= deltaSec;

        if (ripple.life <= 0.0f)
        {
            ripple.active = false;
            continue;
        }

        anyActive = true;
    }

    if (anyActive || autoEmit)
    {
        dirty = true;
    }
}

uint32_t WaterRipple::nextRandom()
{
    randomState = randomState * 1664525u + 1013904223u;
    return randomState;
}

float WaterRipple::randomRange(float minimum, float maximum)
{
    const float unit = static_cast<float>((nextRandom() >> 8) & 0x00FFFFFFu) /
                       static_cast<float>(0x01000000u);
    return minimum + (maximum - minimum) * unit;
}
