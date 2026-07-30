#include "MidiMusicBox.h"
#include "MidiAssets.h"

#include <cstdio>

using namespace PRUZEA;

namespace
{
constexpr int16_t SCREEN_W = 320;
constexpr int16_t SCREEN_H = 240;

struct SongInfo
{
    const char* title;
    const char* subtitle;
};

static const SongInfo SONGS[] = {
    { "FURUSATO", "JAPANESE SONG" },
    { "CANON", "PACHELBEL" },
    { "HIMAWARI", "Mu" }
};

static const Audio::Midi MIDI_SONGS[] = {
    { FURUSATO_MIDI_DATA, FURUSATO_MIDI_SIZE, 0, 0.72f },
    { CANON_MIDI_DATA, CANON_MIDI_SIZE, 0, 0.68f },
    { HIMAWARI_MIDI_DATA, HIMAWARI_MIDI_SIZE, 0, 0.70f }
};

void drawMusicNote(
    Graphics& g,
    int16_t x,
    int16_t y,
    int16_t height,
    Graphics::Color color)
{
    g.fillCircle(x, y, 4, color);
    g.fillRect(x + 3, y - height, 2, height, color);
    g.drawLine(x + 5, y - height, x + 11, y - height + 4, color);
}

} // namespace

const char* MidiMusicBox::getId() const
{
    return "midi_music_box";
}

const char* MidiMusicBox::getName() const
{
    return "MIDI Music Box";
}

const char* MidiMusicBox::getMenuName() const
{
    return "15 MIDI Music Box";
}

uint16_t MidiMusicBox::getLogicalScreenWidth() const { return SCREEN_W; }
uint16_t MidiMusicBox::getLogicalScreenHeight() const { return SCREEN_H; }
uint16_t MidiMusicBox::getTargetScreenWidth() const { return SCREEN_W; }
uint16_t MidiMusicBox::getTargetScreenHeight() const { return SCREEN_H; }

void MidiMusicBox::onInit(Storage& storage)
{
    (void)storage;
    selectedSong = 0;
    playing = false;
    animationStartMsec = Platform::getMsec();
    dirty = true;
}

Game::GameState MidiMusicBox::onUpdate(
    Input& input,
    Audio& audio,
    Storage& storage,
    float deltaSec)
{
    (void)storage;
    (void)deltaSec;

    if (input.justPressed(Input::UP)) selectPrevious(audio);
    if (input.justPressed(Input::DOWN)) selectNext(audio);

    if (input.justPressed(Input::A) ||
        input.justPressed(Input::START))
    {
        if (playing) stop(audio);
        else playSelected(audio);
    }

    if (input.justPressed(Input::B)) stop(audio);

    if (playing) dirty = true;
    return GameState::RUNNING;
}

bool MidiMusicBox::onDraw(
    Graphics& g,
    bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty) return false;

    const Graphics::Color night = Graphics::rgb565(20, 30, 48);
    const Graphics::Color deepBlue = Graphics::rgb565(31, 48, 72);
    const Graphics::Color panel = Graphics::rgb565(42, 61, 84);
    const Graphics::Color gold = Graphics::rgb565(225, 181, 83);
    const Graphics::Color paleGold = Graphics::rgb565(250, 224, 157);
    const Graphics::Color ivory = Graphics::rgb565(245, 238, 218);
    const Graphics::Color muted = Graphics::rgb565(157, 174, 188);
    const Graphics::Color accent = Graphics::rgb565(221, 143, 72);
    const Graphics::Color sunflower = Graphics::rgb565(243, 190, 52);

    g.fillScreen(night);

    // Subtle Japanese seigaiha-inspired arcs.
    for (int16_t x = -16; x < SCREEN_W + 32; x += 32)
    {
        g.drawCircle(x, 232, 24, 12, deepBlue);
        g.drawCircle(x + 16, 232, 24, 12, deepBlue);
    }

    g.drawString(
        "MIDI MUSIC BOX", 160, 12, paleGold, Graphics::SIZE_25B,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    g.drawString(
        "EMBEDDED FORMAT 1", 160, 40, muted, Graphics::SIZE_10,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    g.fillRoundRect(42, 60, 236, 80, 12, panel);
    g.drawRoundRect(42, 60, 236, 80, 12, 2, gold);

    g.fillRoundRect(72, 78, 176, 28, 13, deepBlue);
    g.drawRoundRect(72, 78, 176, 28, 13, 2, paleGold);

    const uint32_t now = Platform::getMsec();
    const uint32_t animation =
        playing ? (now - animationStartMsec) / 80u : 0u;

    for (uint8_t i = 0; i < 11; ++i)
    {
        const int16_t pinX = static_cast<int16_t>(82 + i * 15);
        const int16_t pinY =
            static_cast<int16_t>(90 + ((i + animation) % 3) * 3);
        g.fillCircle(pinX, pinY, 2, gold);
    }

    // A small sunflower motif for HIMAWARI.
    g.fillCircle(260, 70, 9, sunflower);
    g.fillCircle(260, 70, 4, accent);
    for (uint8_t i = 0; i < 8; ++i)
    {
        const float radians = static_cast<float>(i) * 0.785398f;
        const int16_t px =
            static_cast<int16_t>(260 + Math::cos(radians) * 13.0f);
        const int16_t py =
            static_cast<int16_t>(70 + Math::sin(radians) * 13.0f);
        g.fillCircle(px, py, 3, sunflower);
    }

    if (playing)
    {
        drawMusicNote(
            g,
            static_cast<int16_t>(58 + animation % 18),
            static_cast<int16_t>(76 - animation % 8),
            13,
            paleGold);

        drawMusicNote(
            g,
            static_cast<int16_t>(266 - animation % 14),
            static_cast<int16_t>(122 - animation % 10),
            10,
            gold);
    }

    for (uint8_t i = 0; i < SONG_COUNT; ++i)
    {
        const int16_t y = static_cast<int16_t>(151 + i * 24);
        const bool selected = i == selectedSong;

        if (selected)
        {
            g.fillRoundRect(45, y - 5, 230, 22, 6, deepBlue);
            g.drawRoundRect(45, y - 5, 230, 22, 6, 1, gold);
        }

        char number[4];
        std::snprintf(
            number,
            sizeof(number),
            "%02u",
            static_cast<unsigned>(i + 1));

        g.drawString(
            number, 58, y,
            selected ? gold : muted,
            Graphics::SIZE_13,
            Graphics::HorizontalAlign::LEFT,
            Graphics::VerticalAlign::TOP);

        g.drawString(
            SONGS[i].title, 92, y,
            selected ? ivory : muted,
            Graphics::SIZE_13,
            Graphics::HorizontalAlign::LEFT,
            Graphics::VerticalAlign::TOP);

        g.drawString(
            SONGS[i].subtitle, 262, y + 2,
            selected ? paleGold : deepBlue,
            Graphics::SIZE_10,
            Graphics::HorizontalAlign::RIGHT,
            Graphics::VerticalAlign::TOP);
    }

    g.fillRect(0, 224, SCREEN_W, 16, deepBlue);

    dirty = false;
    return true;
}

void MidiMusicBox::onTerminate(Storage& storage)
{
    (void)storage;
}

void MidiMusicBox::selectPrevious(Audio& audio)
{
    selectedSong =
        static_cast<uint8_t>(
            (selectedSong + SONG_COUNT - 1) % SONG_COUNT);

    if (playing) playSelected(audio);
    dirty = true;
}

void MidiMusicBox::selectNext(Audio& audio)
{
    selectedSong =
        static_cast<uint8_t>(
            (selectedSong + 1) % SONG_COUNT);

    if (playing) playSelected(audio);
    dirty = true;
}

void MidiMusicBox::playSelected(Audio& audio)
{
    audio.playMidi(&MIDI_SONGS[selectedSong]);
    playing = true;
    animationStartMsec = Platform::getMsec();
    dirty = true;
}

void MidiMusicBox::stop(Audio& audio)
{
    if (!playing) return;

    audio.stopMusic();
    playing = false;
    dirty = true;
}
