// CyberBeat.h
#pragma once
#include "PLAMIO.h"

namespace PLAMIO
{

// =========================================================================
// # Gameplay Tuning & Balancing Constants (Centralized)
// =========================================================================
constexpr float   BEAT_BPM             = 130.0f;     // Game tempo for neon pulsing
constexpr uint32_t NOTE_SPAWN_INTERVAL = 1200;       // Interval between note spawns (ms)
constexpr uint32_t NOTE_TRAVEL_TIME    = 1500;       // Duration from spawn to judgment line (ms)
constexpr int16_t  JUDGE_LINE_Y        = 180;        // Y position of judgment line
constexpr int16_t  NOTE_RADIUS         = 12;         // Visual size of notes

// Judgment windows (ms)
constexpr uint32_t JUDGE_PERFECT       = 50;         // ±50ms
constexpr uint32_t JUDGE_GREAT         = 120;        // ±120ms
constexpr uint32_t JUDGE_GOOD          = 200;        // ±200ms

constexpr uint16_t MAX_NOTES           = 8;          // Maximum active notes in pool
constexpr uint16_t TARGET_SCORE        = 5000;       // Score required to clear the stage
constexpr int8_t   MAX_LIFE            = 10;         // Initial and maximum player life

class CyberBeat : public Game {
private:
    // ## Internal Game Modes
    enum InternalMode : uint8_t {
        MODE_TITLE,
        MODE_PLAYING,
        MODE_PAUSED,
        MODE_STAGE_CLEAR,
        MODE_GAME_OVER
    };

    // ## Structures
    struct Note {
        uint32_t targetTime; // Absolute timestamp when the note hits judgment line
        int16_t lane;        // 0: LEFT, 1: RIGHT, 2: Y, 3: A
        bool active;
    };

    // ## Game State Variables
    InternalMode m_mode;
    uint32_t m_score;
    uint32_t m_highScore;
    uint32_t m_combo;
    uint32_t m_maxCombo;
    int8_t   m_life;         // Player health counter
    
    // Time management (ms)
    uint32_t m_startTime;
    uint32_t m_lastSpawnTime;
    uint32_t m_lastTime;

    // Object pool
    Note m_notes[MAX_NOTES];

    // Juice & Visual effects variables
    uint32_t m_lastPerfectTime;  // Trigger for screen shake
    uint32_t m_lastMissTime;     // Trigger for red flash overlay
    uint32_t m_juiceTime;        // General animation timestamp
    const char* m_judgeText;     // Stores "PERFECT!", "GREAT", etc.
    uint32_t m_judgeTextTime;    // Timestamp when judgment text appeared

    // ## Private Helper Methods
    void resetGame(uint32_t currentTime);
    void spawnNote(uint32_t currentTime);
    void checkInput(Input& input, Audio& audio, uint32_t currentTime);
    void updateJuice(uint32_t currentTime);
    
public:
    CyberBeat() : m_mode(MODE_TITLE), m_score(0), m_highScore(0), m_combo(0), m_maxCombo(0), m_life(MAX_LIFE),
                  m_startTime(0), m_lastSpawnTime(0), m_lastTime(0), m_lastPerfectTime(0),
                  m_lastMissTime(0), m_juiceTime(0), m_judgeText(""), m_judgeTextTime(0) {}
    virtual ~CyberBeat() override = default;

    // ## PLAMIO System Lifecycle API
    virtual const char* getId() const override { return "cyberbeat"; }
    virtual const char* getName() const override { return "CYBER BEAT"; }
    virtual const char* getMenuName() const override { return "CYBER BEAT"; }

    virtual uint16_t getLogicalScreenWidth() const override { return 320; }
    virtual uint16_t getLogicalScreenHeight() const override { return 240; }
    virtual uint16_t getTargetScreenWidth() const override { return 320; }
    virtual uint16_t getTargetScreenHeight() const override { return 240; }

    void onInit(Storage& storage) override;
    GameState onUpdate(Input& input, Audio& audio, Storage& storage, float deltaSec) override;
    bool onDraw(Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(Storage& storage) override;
};

} // namespace PLAMIO
