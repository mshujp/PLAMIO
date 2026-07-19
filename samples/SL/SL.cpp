#include "SL.h"
#include <cstdlib>
#include <cmath>

namespace PLAMIO
{

static const char* const SL_ART[] = {
    "     ===   ________           _________",
    "   __| |__/ __ _===^_______/_______|",
    "  / _//  H \\____  |  ___    _  |_|_|_|=||",
    " |  _||__H    \\\\__    |           | |  |__| |",
    " |___||__H__|_____|_|___|_|________|_|",
    "    \\__/      \\__/     \\__/      \\__/" 
};
constexpr uint8_t SL_ART_LINES = sizeof(SL_ART) / sizeof(SL_ART[0]);


// Smoke growth stages from small to large
static const char* const SMOKE_TEXTS[] = { "o", "O", "()", "( )" };
constexpr uint8_t SMOKE_TEXT_COUNT = sizeof(SMOKE_TEXTS) / sizeof(SMOKE_TEXTS[0]);

void SL::onInit(Storage& storage) {
    startTimeMsec = Platform::getMsec();
    trainX = 320.0f; 
    isInitialized = true;
    dirty = true;

    for (uint8_t i = 0; i < MAX_SMOKE; ++i) {
        smokeParticles[i].active = false;
    }
    lastSmokeTime = startTimeMsec;
}

void SL::updateSmoke(uint32_t currentMsec) {
    // Spawn a new small smoke particle every 80ms
    if (currentMsec - lastSmokeTime > 80) {
        for (uint8_t i = 0; i < MAX_SMOKE; ++i) {
            if (!smokeParticles[i].active) {
                smokeParticles[i].active = true;
                smokeParticles[i].x = trainX + 48.0f; 
                smokeParticles[i].y = 92.0f;
                smokeParticles[i].stage = 0; // Start with "o"
                smokeParticles[i].animTimer = currentMsec + 200; // Next growth in 200ms
                break;
            }
        }
        lastSmokeTime = currentMsec;
    }

    // Move and grow smoke particles
    for (uint8_t i = 0; i < MAX_SMOKE; ++i) {
        if (smokeParticles[i].active) {
            smokeParticles[i].x -= 1.6f; 
            smokeParticles[i].y -= 0.5f; 

            // Progress to the next growth stage based on time
            if (currentMsec >= smokeParticles[i].animTimer) {
                if (smokeParticles[i].stage < SMOKE_TEXT_COUNT - 1) {            
                    smokeParticles[i].stage++;
                    smokeParticles[i].animTimer = currentMsec + 200;
                }
            }

            if (smokeParticles[i].y < 10 || smokeParticles[i].x < -30) {
                smokeParticles[i].active = false;
            }
        }
    }
}

Game::GameState SL::onUpdate(Input& input, Audio& audio, Storage& storage, float lastSec) {
    if (!isInitialized) return GameState::RUNNING;

    dirty = true;

    uint32_t now = Platform::getMsec();
    uint32_t elapsed = now - startTimeMsec;

    if (elapsed % 350 < 33) {
        audio.playSE(&Audio::SE::NO_10, 0.3f);
    }

    trainX = 320.0f - (static_cast<float>(elapsed) * TRAIN_SPEED);

    updateSmoke(now);

    if (trainX < -TRAIN_WIDTH) {
        return GameState::TERMINATE_REQUEST;
    }

    return GameState::RUNNING;
}

void SL::drawSl(Graphics& graphics, int16_t x, int16_t y) {
    int16_t fontH = graphics.getTextHeight("A", Graphics::Font::SIZE_13);
    
    for (uint8_t i = 0; i < SL_ART_LINES; ++i) {
        graphics.drawString(SL_ART[i], x, y + (i * fontH), Graphics::Color::WHITE, Graphics::Font::SIZE_13);
    }
}

bool SL::onDraw(Graphics& graphics, bool requestFullRedraw) {
    if (!requestFullRedraw && !dirty) {
        return false;
    }

    graphics.fillScreen(Graphics::BLACK);

    // Render growing smoke particles
    for (uint8_t i = 0; i < MAX_SMOKE; ++i) {
        if (smokeParticles[i].active) {
            graphics.drawString(
                SMOKE_TEXTS[smokeParticles[i].stage], 
                static_cast<int16_t>(smokeParticles[i].x), 
                static_cast<int16_t>(smokeParticles[i].y), 
                Graphics::rgb565(128, 128, 128), 
                Graphics::Font::SIZE_18
            );
        }
    }

    drawSl(graphics, static_cast<int16_t>(trainX), 110);

    dirty = false;
    return true;
}

void SL::onTerminate(Storage& storage) {
}

} // namespace PLAMIO
