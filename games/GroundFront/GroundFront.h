// ------------------------------------------------------------
// PLAMIO Sample Game
//
// This sample demonstrates the basic structure and common
// features used when developing games with PLAMIO.
//
// Included features:
//
// - Title screen
// - Main game loop
// - Pause system
// - Ranking save/load
// - Particle effects
// - Enemy spawning
// - Five unique boss battles
// - Bullet patterns
// - Collision detection
// - BGM and sound effects
//
// Use this project as a reference when creating your own
// games with PLAMIO.
// ----------

#pragma once
#include "PLAMIO.h"
#include <cstdint>

class GroundFront final : public PLAMIO::Game {
public:
    GroundFront();

    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;

    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

    void onInit(PLAMIO::Storage& storage) override;
    GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
    enum InternalMode : uint8_t {
        MODE_TITLE,
        MODE_PLAYING,
        MODE_PAUSED,
        MODE_GAME_OVER,
        MODE_STAGE_CLEAR,
        MODE_RANKING
    };

    enum EnemyType : uint8_t {
        ENEMY_BIRD,
        ENEMY_BALLOON,
        ENEMY_TANK,
        ENEMY_DRONE
    };

    struct Player {
        float x;
        float y;
        int16_t hp;
        uint64_t lastShotMsec;
        uint64_t invincibleUntilMsec;
    };

    struct PlayerBullet {
        bool active;
        float x;
        float y;
        float vx;
        float vy;
        uint8_t power;
    };

    struct Enemy {
        bool active;
        EnemyType type;
        float x;
        float y;
        float vx;
        float vy;
        int16_t hp;
        uint64_t bornMsec;
        uint64_t lastShotMsec;
        uint8_t phase;
    };

    struct EnemyBullet {
        bool active;
        float x;
        float y;
        float vx;
        float vy;
        uint8_t radius;
    };

    struct Boss {
        bool active;
        uint8_t type;
        float x;
        float y;
        float targetY;
        int16_t hp;
        int16_t maxHp;
        uint64_t bornMsec;
        uint64_t lastShotMsec;
        uint64_t patternMsec;
        uint8_t phase;
    };

    struct Particle {
        bool active;
        float x;
        float y;
        float vx;
        float vy;
        uint64_t endMsec;
    };

    static constexpr uint8_t MAX_PLAYER_BULLETS = 28;
    static constexpr uint8_t MAX_ENEMIES = 18;
    static constexpr uint8_t MAX_ENEMY_BULLETS = 72;
    static constexpr uint8_t MAX_PARTICLES = 32;
    static constexpr uint8_t RANKING_COUNT = 5;

    InternalMode mode_;
    Player player_;
    PlayerBullet playerBullets_[MAX_PLAYER_BULLETS];
    Enemy enemies_[MAX_ENEMIES];
    EnemyBullet enemyBullets_[MAX_ENEMY_BULLETS];
    Boss boss_;
    Particle particles_[MAX_PARTICLES];

    uint32_t score_;
    uint32_t ranking_[RANKING_COUNT];
    uint8_t currentBossIndex_;
    uint32_t defeatedEnemies_;
    uint32_t escapedEnemies_;
    uint64_t runStartMsec_;
    uint64_t nextBossMsec_;
    uint64_t lastEnemySpawnMsec_;
    uint64_t resultStartMsec_;
    uint64_t shakeUntilMsec_;
    uint64_t flashUntilMsec_;
    uint64_t pauseStartMsec_;
    uint32_t backgroundSeed_;
    bool scoreSaved_;
    bool rankingLoaded_;
    const char* savePath_ = "ranking.txt";

    void resetRun();
    void startRun(PLAMIO::Audio& audio);
    void shiftGameTimers(uint64_t pausedMsec);
    void updatePlaying(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, uint64_t now);
    void updatePlayer(PLAMIO::Input& input, PLAMIO::Audio& audio, uint64_t now);
    void updatePlayerBullets(uint64_t now);
    void updateEnemies(PLAMIO::Audio& audio, uint64_t now);
    void updateEnemyBullets(PLAMIO::Audio& audio, uint64_t now);
    void updateBoss(PLAMIO::Audio& audio, PLAMIO::Storage& storage, uint64_t now);
    void updateParticles(uint64_t now);

    void spawnEnemy(uint64_t now);
    void spawnBoss(uint64_t now, PLAMIO::Audio& audio);
    void firePlayerShot(PLAMIO::Audio& audio);
    void fireEnemyAimed(float x, float y, float speed);
    void fireEnemyFan(float x, float y, uint8_t count, float speed, float spread);
    void fireEnemyRing(float x, float y, uint8_t count, float speed, float angleOffset);
    void addPlayerBullet(float x, float y, float vx, float vy, uint8_t power);
    void addEnemyBullet(float x, float y, float vx, float vy, uint8_t radius);
    void addExplosion(float x, float y, uint8_t count, uint64_t now);

    bool hitCircle(float ax, float ay, float ar, float bx, float by, float br) const;
    void damagePlayer(PLAMIO::Audio& audio, uint64_t now);
    void finishRun(bool cleared, PLAMIO::Audio& audio, PLAMIO::Storage& storage, uint64_t now);

    void loadRanking(PLAMIO::Storage& storage);
    void registerScore();
    void saveRanking(PLAMIO::Storage& storage);

    void drawBackground(PLAMIO::Graphics& graphics, uint64_t now);
    void drawSidePanels(PLAMIO::Graphics& graphics, uint64_t now);
    void drawTitle(PLAMIO::Graphics& graphics, uint64_t now);
    void drawPlayfield(PLAMIO::Graphics& graphics, uint64_t now);
    void drawPlayer(PLAMIO::Graphics& graphics, uint64_t now);
    void drawEnemies(PLAMIO::Graphics& graphics, uint64_t now);
    void drawBoss(PLAMIO::Graphics& graphics, uint64_t now);
    void drawBullets(PLAMIO::Graphics& graphics);
    void drawParticles(PLAMIO::Graphics& graphics, uint64_t now);
    void drawPause(PLAMIO::Graphics& graphics);
    void drawResult(PLAMIO::Graphics& graphics, uint64_t now);
    void drawRanking(PLAMIO::Graphics& graphics, uint64_t now);
};
