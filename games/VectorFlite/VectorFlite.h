// VectorFlite.h
#pragma once
#include "PLAMIO.h"

namespace PLAMIO
{

class VectorFlite : public Game {
public:
    // --- 構造体・定数定義 ---
    static constexpr int MAX_ASTEROIDS = 8;
    static constexpr int MAX_BULLETS = 10;
    static constexpr int MAX_PARTICLES = 20;

    struct Entity {
        float x, y;
        float vx, vy;
        float radius;
        float angle;
        bool active;
    };

    struct Bullet : public Entity {
        float targetAngle; // For homing bullets (Math::moveTowardsAngle test)
        uint32_t spawnTime;
    };

    struct Particle {
        float x, y;
        float vx, vy;
        uint16_t color;
        float life; // 1.0 down to 0.0
        bool active;
    };

    // --- Gameクラスの仮想関数オーバーライド ---
    void onInit(Storage& storage) override;
    Game::GameState onUpdate(Input& input, Audio& audio, Storage& storage, float deltaSec) override;
    bool onDraw(Graphics& graphics, bool requestFullRedraw) override;
    void onTerminate(Storage& storage) override;

    const char* getId() const override { return "vectorflite"; }
    const char* getName() const override { return "VECTOR FLITE"; }
    const char* getMenuName() const override { return "VECTOR FLITE"; }

    uint16_t getLogicalScreenWidth() const override { return 320; }
    uint16_t getLogicalScreenHeight() const override { return 240; }
    uint16_t getTargetScreenWidth() const override { return 320; }
    uint16_t getTargetScreenHeight() const override { return 240; }

private:
    // --- 内部状態管理用の列挙型 ---
    enum Mode {
        MODE_TITLE,
        MODE_PLAYING,
        MODE_GAME_OVER
    };

    // --- ゲームシステム変数 ---
    Mode currentMode = MODE_TITLE;
    uint32_t score = 0;
    uint32_t highScore = 0;
    float screenShakeTimer = 0.0f; // 画面シェイク時間

    // --- エンティティデータ ---
    Entity player;
    Bullet bullets[MAX_BULLETS];
    Entity asteroids[MAX_ASTEROIDS];
    Particle particles[MAX_PARTICLES];

    // --- 演出・タイマー用変数 ---
    uint32_t stateStartTime = 0;
    float lastShotTime = 0.0f;
    float nextSpawnTime = 0.0f;

    // --- 内部ヘルパー関数 ---
    void resetGame();
    void spawnAsteroid();
    void spawnExplosion(float x, float y, uint16_t color);
    void updateHoming(float deltaSec);
};

} // namespace PLAMIO