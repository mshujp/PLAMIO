// -----------------------------------------------------------------------------
// ParticleLab
//
// PLAMIO Graphics API Sample
//
// Demonstrates:
//
//   * Fixed-size particle pools
//   * Velocity and lifetime
//   * Effect-specific spawn and update rules
//   * Simple particle rendering with the Graphics API
//
// Use these small patterns as a starting point for effects in your own games.
// Particle behavior is intentionally kept in the game instead of being
// generalized into a framework-level particle system.
// -----------------------------------------------------------------------------

#pragma once
#include "PLAMIO.h"

class ParticleLab : public PLAMIO::Game
{
public:
    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

protected:
    void onInit(PLAMIO::Storage& storage) override;
    GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio,
                       PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    enum class Mode : uint8_t
    {
        TITLE,
        LAB
    };

public:
    enum class Effect : uint8_t
    {
        EXPLOSION,
        SMOKE,
        FOUNTAIN,
        SNOW,
        COUNT
    };

private:
    struct Particle
    {
        float x;
        float y;
        float vx;
        float vy;
        float life;
        float maxLife;
        float size;
        float phase;
        PLAMIO::Graphics::Color color;
        bool active;
    };

    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;
    static constexpr uint8_t MAX_PARTICLES = 80;

    Mode mode = Mode::TITLE;
    Effect effect = Effect::EXPLOSION;
    Particle particles[MAX_PARTICLES] = {};

    bool autoEmit = true;
    uint32_t lastEmitMsec = 0;
    uint32_t titleBlinkMsec = 0;
    bool showPressStart = true;

    void clearParticles();
    Particle* acquireParticle();
    uint8_t activeParticleCount() const;

    void selectEffect(int8_t direction, PLAMIO::Audio& audio);
    void emitSelected();
    void updateParticles(float deltaSec);

    void spawnExplosion();
    void spawnSmoke(uint8_t count);
    void spawnFountain(uint8_t count);
    void spawnSnow(uint8_t count);

    void updateExplosion(Particle& particle, float deltaSec);
    void updateSmoke(Particle& particle, float deltaSec);
    void updateFountain(Particle& particle, float deltaSec);
    void updateSnow(Particle& particle, float deltaSec);

    void drawTitle(PLAMIO::Graphics& graphics);
    void drawLab(PLAMIO::Graphics& graphics);
    void drawParticle(PLAMIO::Graphics& graphics, const Particle& particle);
};
