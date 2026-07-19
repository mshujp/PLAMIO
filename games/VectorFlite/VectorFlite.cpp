// VectorFlite.cpp
#include "VectorFlite.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace PLAMIO
{

// --- ユーティリティ・定数定義（ゲームバランス調整用の定数群） ---
static const float PLAYER_ROT_SPEED = 4.0f;     // 自機の回転速度（ラジアン/秒）
static const float PLAYER_ACCEL = 180.0f;       // 自機の加速力
static const float PLAYER_MAX_SPEED = 150.0f;   // 自機の最高速度限制
static const float PLAYER_FRICTION = 0.98f;     // 宇宙摩擦
static const float BULLET_SPEED = 240.0f;       // 弾の基本速度
static const float HOMING_STRENGTH = 5.0f;      // 誘導弾の旋回力（ラジアン/秒）
static const float ASTEROID_MIN_SPEED = 30.0f;  // 隕石の最小速度
static const float ASTEROID_MAX_SPEED = 70.0f;  // 隕石の最大速度
static const float SHAKE_DECAY = 5.0f;          // 画面シェイクの減衰速度

// 弾の消滅ライフ（ミリ秒）
static const uint32_t BULLET_LIFE_MS = 1500;

// カラー戦略（Sci-Fiテーマ: グレイ調の背景、サイアンとマゼンタのネオンカラー）
static const Graphics::Color COLOR_BG       = Graphics::BLACK;
static const Graphics::Color COLOR_GRID     = Graphics::rgb565(20, 24, 40);   // 暗い紺
static const Graphics::Color COLOR_PLAYER   = Graphics::CYAN;                // サイアン
static const Graphics::Color COLOR_BULLET   = Graphics::YELLOW;
static const Graphics::Color COLOR_ASTEROID = Graphics::MAGENTA;             // マゼンタ
static const Graphics::Color COLOR_HUD      = Graphics::WHITE;
static const Graphics::Color COLOR_ALERT    = Graphics::RED;

// --- 自機のワイヤーフレームモデル用頂点（回転前） ---
struct Vec2 { float x, y; };
static const Vec2 SHIP_MODEL[] = {
    { 12.0f,  0.0f }, // 機首
    {-8.0f,  -6.0f }, // 左後
    {-5.0f,   0.0f }, // 後部凹み
    {-8.0f,   6.0f }  // 右後
};
static const int SHIP_VERTEX_COUNT = 4;

// --- 初期化処理 ---
void VectorFlite::onInit(Storage& storage)
{
    // ハイスコアの読み込み（UserFileの軽量API）
    char highSave[32];
    if (storage.readUserFile(getId(), "highscore.dat", [](const char* line, void* arg) -> bool {
        auto* self = static_cast<VectorFlite*>(arg);
        self->highScore = std::atoi(line);
        return false; // 1行だけ読む
    }, this)) {
        // 読み込み成功
    } else {
        highScore = 0;
    }

    currentMode = MODE_TITLE;
    resetGame();
}

// --- ゲームリセット処理 ---
void VectorFlite::resetGame()
{
    score = 0;
    screenShakeTimer = 0.0f;

    // プレイヤー初期化
    player.x = 160.0f;
    player.y = 120.0f;
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.radius = 6.0f;
    player.angle = -1.5707f; // 上向き (-PI/2)
    player.active = true;

    // 弾配列の初期化
    for (int i = 0; i < MAX_BULLETS; ++i) {
        bullets[i].active = false;
    }

    // 小惑星の初期化
    for (int i = 0; i < MAX_ASTEROIDS; ++i) {
        asteroids[i].active = false;
    }

    // パーティクルの初期化
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        particles[i].active = false;
    }

    lastShotTime = 0.0f;
    nextSpawnTime = 0.0f;
    stateStartTime = Platform::getMsec();
}

// --- 小惑星の生成 ---
void VectorFlite::spawnAsteroid()
{
    for (int i = 0; i < MAX_ASTEROIDS; ++i) {
        if (!asteroids[i].active) {
            // 画面外のランダムな位置に生成
            int edge = std::rand() % 4;
            float sx = 0, sy = 0;
            switch (edge) {
                case 0: sx = (float)(std::rand() % 320); sy = -20.0f; break; // 上
                case 1: sx = (float)(std::rand() % 320); sy = 260.0f; break; // 下
                case 2: sx = -20.0f; sy = (float)(std::rand() % 240); break; // 左
                case 3: sx = 340.0f; sy = (float)(std::rand() % 240); break; // 右
            }

            // 自機に向かうベクトルを計算し、角度にランダムな揺らぎを加える
            float targetX = player.x;
            float targetY = player.y;
            float dx = targetX - sx;
            float dy = targetY - sy;
            
            // Math::normalize を使用して進行方向ベクトルを得る
            Math::normalize(dx, dy);
            
            // 速度を設定
            float speed = ASTEROID_MIN_SPEED + (float)(std::rand() % (int)(ASTEROID_MAX_SPEED - ASTEROID_MIN_SPEED));
            asteroids[i].x = sx;
            asteroids[i].y = sy;
            asteroids[i].vx = dx * speed;
            asteroids[i].vy = dy * speed;
            asteroids[i].radius = 12.0f + (std::rand() % 8); // サイズランダム
            asteroids[i].angle = (float)(std::rand() % 314) / 100.0f;
            asteroids[i].active = true;
            break;
        }
    }
}

// --- 爆発エフェクトの生成 ---
void VectorFlite::spawnExplosion(float x, float y, uint16_t color)
{
    int pCount = 0;
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!particles[i].active) {
            // 放射状にパーティクルを飛ばす
            float angle = (float)(std::rand() % 628) / 100.0f;
            float speed = 30.0f + (std::rand() % 50);
            
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = Math::cos(angle) * speed;
            particles[i].vy = Math::sin(angle) * speed;
            particles[i].color = color;
            particles[i].life = 1.0f;
            particles[i].active = true;

            pCount++;
            if (pCount >= 5) break; // 1回の爆発につき最大5つ
        }
    }
}

// --- 誘導弾の角度補間処理 ---
void VectorFlite::updateHoming(float deltaSec)
{
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;

        // 最も近いアクティブな小惑星を探す
        float minDistSq = 999999.0f;
        int targetIdx = -1;

        for (int j = 0; j < MAX_ASTEROIDS; ++j) {
            if (!asteroids[j].active) continue;
            
            // Math::distanceSquared を使用して最短距離を計算
            float distSq = Math::distanceSquared(bullets[i].x, bullets[i].y, asteroids[j].x, asteroids[j].y);
            if (distSq < minDistSq) {
                minDistSq = distSq;
                targetIdx = j;
            }
        }

        // ロックオン対象がある場合、誘導角度を補間
        if (targetIdx != -1) {
            float dx = asteroids[targetIdx].x - bullets[i].x;
            float dy = asteroids[targetIdx].y - bullets[i].y;
            
            // Math::angle (atan2f) でターゲットの角度を計算
            float targetAngle = Math::angle(dx, dy);

            // Math::moveTowardsAngle でスムーズに誘導
            bullets[i].angle = Math::moveTowardsAngle(bullets[i].angle, targetAngle, HOMING_STRENGTH * deltaSec);

            // 速度ベクトルを更新
            bullets[i].vx = Math::cos(bullets[i].angle) * BULLET_SPEED;
            bullets[i].vy = Math::sin(bullets[i].angle) * BULLET_SPEED;
        }
    }
}

// --- フレーム更新処理 ---
Game::GameState VectorFlite::onUpdate(Input& input, Audio& audio, Storage& storage, float deltaSec)
{
    uint32_t now = Platform::getMsec();

    // 画面シェイクのタイマー減衰処理 (Math::moveTowards の活用)
    if (screenShakeTimer > 0.0f) {
        screenShakeTimer = Math::moveTowards(screenShakeTimer, 0.0f, SHAKE_DECAY * deltaSec);
    }

    switch (currentMode) {
        case MODE_TITLE:
            if (input.justPressed(Input::A) || input.justPressed(Input::START)) {
                audio.playSE(&Audio::SE::NO_8, 1.0f); // 開始ファンファーレ
                resetGame();
                currentMode = MODE_PLAYING;
                dirty = true;
            }
            break;

        case MODE_PLAYING:
            // --- 1. 自機の回転と移動制御 ---
            if (input.pressed(Input::LEFT)) {
                player.angle -= PLAYER_ROT_SPEED * deltaSec;
                dirty = true;
            }
            if (input.pressed(Input::RIGHT)) {
                player.angle += PLAYER_ROT_SPEED * deltaSec;
                dirty = true;
            }

            // Aボタンで加速（慣性飛行）
            if (input.pressed(Input::A)) {
                float ax = Math::cos(player.angle) * PLAYER_ACCEL * deltaSec;
                float ay = Math::sin(player.angle) * PLAYER_ACCEL * deltaSec;
                player.vx += ax;
                player.vy += ay;

                // Math::length による最高速度制限
                float speed = Math::length(player.vx, player.vy);
                if (speed > PLAYER_MAX_SPEED) {
                    Math::normalize(player.vx, player.vy);
                    player.vx *= PLAYER_MAX_SPEED;
                    player.vy *= PLAYER_MAX_SPEED;
                }
                dirty = true;
            } else {
                // 加速していないときは緩やかに摩擦減速
                player.vx *= Math::lerp(1.0f, PLAYER_FRICTION, deltaSec * 60.0f);
                player.vy *= Math::lerp(1.0f, PLAYER_FRICTION, deltaSec * 60.0f);
            }

            // 自機の移動更新
            player.x += player.vx * deltaSec;
            player.y += player.vy * deltaSec;

            // Math::wrap による宇宙空間の無限スクリードループ処理
            player.x = Math::wrap(player.x, 0.0f, 320.0f);
            player.y = Math::wrap(player.y, 0.0f, 240.0f);

            // --- 2. 射撃制御 (Bボタン) ---
            if (input.pressed(Input::B)) {
                float timeSinceLastShot = (float)(now - lastShotTime) / 1000.0f;
                if (timeSinceLastShot >= 0.2f) { // 連射制限
                    for (int i = 0; i < MAX_BULLETS; ++i) {
                        if (!bullets[i].active) {
                            bullets[i].x = player.x + Math::cos(player.angle) * 10.0f;
                            bullets[i].y = player.y + Math::sin(player.angle) * 10.0f;
                            bullets[i].vx = Math::cos(player.angle) * BULLET_SPEED;
                            bullets[i].vy = Math::sin(player.angle) * BULLET_SPEED;
                            bullets[i].angle = player.angle;
                            bullets[i].radius = 2.0f;
                            bullets[i].spawnTime = now;
                            bullets[i].active = true;

                            audio.playSE(&Audio::SE::NO_1, 0.6f); // レーザー
                            lastShotTime = now;
                            break;
                        }
                    }
                }
            }

            // --- 3. 誘導＆弾の移動更新 ---
            updateHoming(deltaSec);

            for (int i = 0; i < MAX_BULLETS; ++i) {
                if (bullets[i].active) {
                    bullets[i].x += bullets[i].vx * deltaSec;
                    bullets[i].y += bullets[i].vy * deltaSec;

                    // 弾の画面ループ
                    bullets[i].x = Math::wrap(bullets[i].x, 0.0f, 320.0f);
                    bullets[i].y = Math::wrap(bullets[i].y, 0.0f, 240.0f);

                    // 寿命での消滅判定
                    if (now - bullets[i].spawnTime > BULLET_LIFE_MS) {
                        bullets[i].active = false;
                    }
                    dirty = true;
                }
            }

            // --- 4. 小惑星の生成と移動更新 ---
            if ((float)now / 1000.0f >= nextSpawnTime) {
                spawnAsteroid();
                nextSpawnTime = ((float)now / 1000.0f) + 1.5f - (float)Math::clamp((float)score / 2000.0f, 0.0f, 1.0f); // スコアが上がると生成ペースUP
            }

            for (int i = 0; i < MAX_ASTEROIDS; ++i) {
                if (asteroids[i].active) {
                    asteroids[i].x += asteroids[i].vx * deltaSec;
                    asteroids[i].y += asteroids[i].vy * deltaSec;
                    asteroids[i].angle += 1.0f * deltaSec; // 自転

                    // 画面外に大きく出たらリセット
                    if (asteroids[i].x < -30.0f || asteroids[i].x > 350.0f ||
                        asteroids[i].y < -30.0f || asteroids[i].y > 270.0f) {
                        asteroids[i].active = false;
                    }
                    dirty = true;
                }
            }

            // --- 5. パーティクルの移動更新 ---
            for (int i = 0; i < MAX_PARTICLES; ++i) {
                if (particles[i].active) {
                    particles[i].x += particles[i].vx * deltaSec;
                    particles[i].y += particles[i].vy * deltaSec;
                    particles[i].life -= 2.0f * deltaSec; // 急速にフェード
                    if (particles[i].life <= 0.0f) {
                        particles[i].active = false;
                    }
                    dirty = true;
                }
            }

            // --- 6. 衝突判定（Math::distanceSquared をフル活用） ---
            for (int i = 0; i < MAX_ASTEROIDS; ++i) {
                if (!asteroids[i].active) continue;

                // A. 弾 vs 小惑星
                for (int j = 0; j < MAX_BULLETS; ++j) {
                    if (!bullets[j].active) continue;

                    // 高速な二乗距離比較（sqrtfをバイパス）
                    float limitDist = asteroids[i].radius + bullets[j].radius;
                    float distSq = Math::distanceSquared(asteroids[i].x, asteroids[i].y, bullets[j].x, bullets[j].y);

                    if (distSq < limitDist * limitDist) {
                        // 命中
                        spawnExplosion(asteroids[i].x, asteroids[i].y, COLOR_ASTEROID);
                        audio.playSE(&Audio::SE::NO_6, 0.8f); // 撃破音
                        
                        asteroids[i].active = false;
                        bullets[j].active = false;
                        
                        score += 100;
                        screenShakeTimer = 0.25f; // 小規模シェイク
                        dirty = true;
                        break;
                    }
                }

                // B. 自機 vs 小惑星
                if (asteroids[i].active) {
                    float limitDist = asteroids[i].radius + player.radius;
                    float distSq = Math::distanceSquared(asteroids[i].x, asteroids[i].y, player.x, player.y);

                    if (distSq < limitDist * limitDist) {
                        // 衝突・ゲームオーバー
                        spawnExplosion(player.x, player.y, COLOR_PLAYER);
                        audio.playSE(&Audio::SE::NO_4, 1.0f); // 爆発音
                        screenShakeTimer = 0.6f;             // 強力な画面シェイク
                        
                        // ハイスコア更新チェック＆保存
                        if (score > highScore) {
                            highScore = score;
                            char buf[32];
                            std::snprintf(buf, sizeof(buf), "%d", highScore);
                            storage.writeUserFile(getId(), "highscore.dat", buf);
                        }

                        currentMode = MODE_GAME_OVER;
                        stateStartTime = now;
                        dirty = true;
                    }
                }
            }
            break;

        case MODE_GAME_OVER:
            if (now - stateStartTime > 1500) { // 1.5秒経過後
                if (input.justPressed(Input::A) || input.justPressed(Input::START)) {
                    audio.playSE(&Audio::SE::NO_5, 1.0f);
                    resetGame();
                    currentMode = MODE_PLAYING;
                    dirty = true;
                }
            }
            break;
    }

    return GameState::RUNNING;
}

// --- 画面描画処理 ---
bool VectorFlite::onDraw(Graphics& graphics, bool requestFullRedraw)
{
    // 描画抑制チェック
    if (!requestFullRedraw && !dirty) {
        return false;
    }

    // 画面シェイク処理（イージング・Tween調に適用）
    if (screenShakeTimer > 0.0f) {
        // 最大±8ピクセルの範囲でランダムオフセット
        int16_t offset_x = (std::rand() % 17) - 8;
        int16_t offset_y = (std::rand() % 17) - 8;
        graphics.setViewport(offset_x, offset_y);
    } else {
        graphics.setViewport(0, 0);
    }

    // 画面クリア
    graphics.fillScreen(COLOR_BG);

    // --- 背景：グリッド表示（Sci-Fiテーマ演出） ---
    for (int x = 0; x < 320; x += 40) {
        graphics.drawLine(x, 0, x, 240, COLOR_GRID);
    }
    for (int y = 0; y < 240; y += 40) {
        graphics.drawLine(0, y, 320, y, COLOR_GRID);
    }

    // 各モードの描画処理
    if (currentMode == MODE_TITLE) {
        // タイトル
        graphics.drawString("VECTOR FLITE", 160, 80, COLOR_PLAYER, Graphics::SIZE_32B, Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
        
        // 点滅する案内文字
        if ((Platform::getMsec() / 400) % 2 == 0) {
            graphics.drawString("PRESS START BUTTON", 160, 150, COLOR_HUD, Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
        }

        char highStr[64];
        std::snprintf(highStr, sizeof(highStr), "HIGH SCORE: %d", highScore);
        graphics.drawString(highStr, 160, 190, COLOR_BULLET, Graphics::SIZE_13, Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);

    } else {
        // --- プレイ中 / ゲームオーバーのゲーム描画 ---

        // 1. パーティクルの描画
        for (int i = 0; i < MAX_PARTICLES; ++i) {
            if (particles[i].active) {
                graphics.drawPixel((int16_t)particles[i].x, (int16_t)particles[i].y, (Graphics::Color)particles[i].color);
            }
        }

        // 2. 弾の描画
        for (int i = 0; i < MAX_BULLETS; ++i) {
            if (bullets[i].active) {
                graphics.fillCircle((int16_t)bullets[i].x, (int16_t)bullets[i].y, (uint16_t)bullets[i].radius, COLOR_BULLET);
            }
        }

        // 3. 小惑星の描画（ワイヤーフレーム調の円）
        for (int i = 0; i < MAX_ASTEROIDS; ++i) {
            if (asteroids[i].active) {
                // 中心にコア
                graphics.fillCircle((int16_t)asteroids[i].x, (int16_t)asteroids[i].y, 2, COLOR_ASTEROID);
                // 外殻
                graphics.drawCircle((int16_t)asteroids[i].x, (int16_t)asteroids[i].y, (uint16_t)asteroids[i].radius, COLOR_ASTEROID);
                
                // 角度線
                float radX = Math::cos(asteroids[i].angle) * asteroids[i].radius;
                float radY = Math::sin(asteroids[i].angle) * asteroids[i].radius;
                graphics.drawLine((int16_t)asteroids[i].x, (int16_t)asteroids[i].y, 
                                  (int16_t)(asteroids[i].x + radX), (int16_t)(asteroids[i].y + radY), COLOR_ASTEROID);

                // --- Math::dot（内積）による正面警告アラート機能のデモ ---
                // 自機の正面ベクトル
                float faceX = Math::cos(player.angle);
                float faceY = Math::sin(player.angle);
                // 敵（小惑星）への相対ベクトル
                float toTargetX = asteroids[i].x - player.x;
                float toTargetY = asteroids[i].y - player.y;
                Math::normalize(toTargetX, toTargetY);

                // 内積（dot product）を取得
                float dotVal = Math::dot(faceX, faceY, toTargetX, toTargetY);
                // 正面約60度（cos(30deg) ≒ 0.866）以内に小惑星が接近している場合にHUDに警告マーカー
                if (dotVal > 0.866f && player.active) {
                    float dist = Math::distance(player.x, player.y, asteroids[i].x, asteroids[i].y);
                    if (dist < 100.0f) {
                        // 正面近接警告ライン
                        graphics.drawCircle((int16_t)asteroids[i].x, (int16_t)asteroids[i].y, (uint16_t)(asteroids[i].radius + 4), COLOR_ALERT);
                    }
                }
            }
        }

        // 4. 自機（三角錐ワイヤーフレーム）の回転・描画
        if (currentMode == MODE_PLAYING) {
            Vec2 rotatedModel[SHIP_VERTEX_COUNT];
            for (int v = 0; v < SHIP_VERTEX_COUNT; ++v) {
                float rx, ry;
                // Math::rotate を使ってモデルデータを自機の角度に回転
                Math::rotate(SHIP_MODEL[v].x, SHIP_MODEL[v].y, player.angle, rx, ry);
                rotatedModel[v].x = player.x + rx;
                rotatedModel[v].y = player.y + ry;
            }

            // 輪郭線の描画
            for (int v = 0; v < SHIP_VERTEX_COUNT; ++v) {
                int next = (v + 1) % SHIP_VERTEX_COUNT;
                graphics.drawLine((int16_t)rotatedModel[v].x, (int16_t)rotatedModel[v].y,
                                  (int16_t)rotatedModel[next].x, (int16_t)rotatedModel[next].y, COLOR_PLAYER);
            }
        }

        // 5. HUDの表示
        char scoreStr[32];
        std::snprintf(scoreStr, sizeof(scoreStr), "SCORE: %d", score);
        graphics.drawString(scoreStr, 10, 10, COLOR_HUD, Graphics::SIZE_13);

        // ゲームオーバー時のオーバーレイ
        if (currentMode == MODE_GAME_OVER) {
            graphics.drawString("GAME OVER", 160, 100, COLOR_ALERT, Graphics::SIZE_25B, Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
            
            if ((Platform::getMsec() / 400) % 2 == 0) {
                graphics.drawString("PRESS START TO RETRY", 160, 150, COLOR_HUD, Graphics::SIZE_18, Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
            }
        }
    }

    // 描画更新完了フラグを戻す
    dirty = false;
    return true;
}

// --- 終了処理 ---
void VectorFlite::onTerminate(Storage& storage)
{
    // 特になし（セーブはリアルタイムで行っています）
}

} // namespace PLAMIO