#include "SpriteAdventure.h"

#include <cstdio>
#include <cmath>

using namespace PLAMIO;

namespace
{
constexpr Graphics::Color TRANSPARENT = Graphics::MAGENTA;

#define T TRANSPARENT
#define K Graphics::BLACK
#define W Graphics::WHITE
#define B Graphics::BLUE
#define C Graphics::CYAN
#define Y Graphics::YELLOW
#define G Graphics::GREEN
#define R Graphics::RED

const uint16_t PLAYER_IDLE[8 * 8] = {
    T,T,K,K,K,K,T,T,
    T,K,W,W,W,W,K,T,
    K,W,B,B,B,B,W,K,
    K,B,B,B,B,B,B,K,
    T,K,B,Y,Y,B,K,T,
    T,K,B,B,B,B,K,T,
    T,K,T,K,K,T,K,T,
    K,T,T,T,T,T,T,K
};

const uint16_t PLAYER_WALK_1[8 * 8] = {
    T,T,K,K,K,K,T,T,
    T,K,W,W,W,W,K,T,
    K,W,B,B,B,B,W,K,
    K,B,B,B,B,B,B,K,
    T,K,B,Y,Y,B,K,T,
    T,K,B,B,B,B,K,T,
    T,T,K,K,T,K,T,T,
    T,K,T,T,K,T,T,T
};

const uint16_t PLAYER_WALK_2[8 * 8] = {
    T,T,K,K,K,K,T,T,
    T,K,W,W,W,W,K,T,
    K,W,B,B,B,B,W,K,
    K,B,B,B,B,B,B,K,
    T,K,B,Y,Y,B,K,T,
    T,K,B,B,B,B,K,T,
    T,K,T,K,K,T,T,T,
    T,T,K,T,T,K,T,T
};

const uint16_t GEM_SPRITE[8 * 8] = {
    T,T,T,C,C,T,T,T,
    T,T,C,W,W,C,T,T,
    T,C,W,C,C,W,C,T,
    C,W,C,C,C,C,W,C,
    C,W,C,C,C,C,W,C,
    T,C,W,C,C,W,C,T,
    T,T,C,W,W,C,T,T,
    T,T,T,C,C,T,T,T
};

#undef T
#undef K
#undef W
#undef B
#undef C
#undef Y
#undef G
#undef R

constexpr float GEM_POSITIONS[6] = {
    150.0f, 302.0f, 468.0f, 650.0f, 828.0f, 1035.0f
};
}

void SpriteAdventure::onInit(Storage& storage)
{
    (void)storage;
    resetAdventure();
    mode = Mode::TITLE;
    dirty = true;
}

Game::GameState SpriteAdventure::onUpdate(
    Input& input,
    Audio& audio,
    Storage& storage,
    float deltaSec)
{
    (void)storage;

    if (mode == Mode::TITLE)
    {
        if (input.justPressed(Input::A) || input.justPressed(Input::START))
        {
            resetAdventure();
            mode = Mode::PLAYING;
            audio.playSE(&Audio::SE::NO_8, 0.7f);
            dirty = true;
        }
        return GameState::RUNNING;
    }

    if (mode == Mode::COMPLETE)
    {
        if (Platform::elapsed(
                Platform::getMsec(),
                completeMsec,
                COMPLETE_WAIT_MSEC) &&
            (input.justPressed(Input::A) ||
             input.justPressed(Input::START)))
        {
            resetAdventure();
            mode = Mode::PLAYING;
            audio.playSE(&Audio::SE::NO_8, 0.7f);
            dirty = true;
        }
        return GameState::RUNNING;
    }

    if (input.justPressed(Input::START))
    {
        mode = Mode::TITLE;
        audio.playSE(&Audio::SE::NO_2, 0.6f);
        dirty = true;
        return GameState::RUNNING;
    }

    updatePlayer(input, audio, deltaSec);
    updateCamera();
    collectGems(audio);

    return GameState::RUNNING;
}

bool SpriteAdventure::onDraw(
    Graphics& graphics,
    bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty)
    {
        return false;
    }

    if (mode == Mode::TITLE)
    {
        drawTitle(graphics);
    }
    else
    {
        drawWorld(graphics);

        if (mode == Mode::COMPLETE)
        {
            graphics.fillRoundRect(
                37, 78, 256, 72, 8,
                Graphics::rgb565(8, 24, 38));

            graphics.drawRoundRect(
                37, 78, 256, 72, 8, 2,
                Graphics::CYAN);

            graphics.drawString(
                "ALL GEMS COLLECTED!",
                SCREEN_W / 2,
                93,
                Graphics::YELLOW,
                Graphics::SIZE_18,
                Graphics::HorizontalAlign::CENTER,
                Graphics::VerticalAlign::TOP);

            graphics.drawString(
                "A / START : RETRY",
                SCREEN_W / 2,
                124,
                Graphics::WHITE,
                Graphics::SIZE_13,
                Graphics::HorizontalAlign::CENTER,
                Graphics::VerticalAlign::TOP);
        }
    }

    dirty = false;
    return true;
}

void SpriteAdventure::onTerminate(Storage& storage)
{
    (void)storage;
}

void SpriteAdventure::resetAdventure()
{
    playerX = 36.0f;
    playerY = static_cast<float>(GROUND_Y - DRAW_H);
    cameraX = 0.0f;

    facingLeft = false;
    walking = false;
    walkFrame = 0;
    lastWalkFrameMsec = Platform::getMsec();

    collectedCount = 0;

    for (uint8_t i = 0; i < GEM_COUNT; ++i)
    {
        gems[i].x = GEM_POSITIONS[i];
        gems[i].y = static_cast<float>(GROUND_Y - DRAW_H - 3);
        gems[i].collected = false;
    }

    dirty = true;
}

void SpriteAdventure::updatePlayer(
    Input& input,
    Audio& audio,
    float deltaSec)
{
    float direction = 0.0f;

    if (input.pressed(Input::LEFT))
    {
        direction -= 1.0f;
    }

    if (input.pressed(Input::RIGHT))
    {
        direction += 1.0f;
    }

    walking = direction != 0.0f;

    if (walking)
    {
        facingLeft = direction < 0.0f;
        playerX += direction * PLAYER_SPEED * deltaSec;
        playerX = Math::clamp(
            playerX,
            0.0f,
            WORLD_W - DRAW_W);

        const uint32_t now = Platform::getMsec();
        if (Platform::elapsed(
                now,
                lastWalkFrameMsec,
                WALK_FRAME_MSEC))
        {
            walkFrame ^= 1;
            lastWalkFrameMsec = now;
        }

        dirty = true;
    }
    else
    {
        walkFrame = 0;
    }

    if (input.justPressed(Input::B))
    {
        resetAdventure();
        audio.playSE(&Audio::SE::NO_2, 0.5f);
    }
}

void SpriteAdventure::updateCamera()
{
    // The world can be wider than the screen without allocating a larger canvas.
    cameraX = playerX + DRAW_W * 0.5f - SCREEN_W * 0.5f;
    cameraX = Math::clamp(
        cameraX,
        0.0f,
        WORLD_W - SCREEN_W);
}

void SpriteAdventure::collectGems(Audio& audio)
{
    const float playerCenterX = playerX + DRAW_W * 0.5f;
    const float playerCenterY = playerY + DRAW_H * 0.5f;

    for (uint8_t i = 0; i < GEM_COUNT; ++i)
    {
        if (gems[i].collected)
        {
            continue;
        }

        const float gemCenterX = gems[i].x + DRAW_W * 0.5f;
        const float gemCenterY = gems[i].y + DRAW_H * 0.5f;

        if (Collision::circleCircle(
                playerCenterX,
                playerCenterY,
                PLAYER_RADIUS,
                gemCenterX,
                gemCenterY,
                6.0f))
        {
            gems[i].collected = true;
            ++collectedCount;
            audio.playSE(&Audio::SE::NO_11, 0.65f);
            dirty = true;

            if (collectedCount == GEM_COUNT)
            {
                mode = Mode::COMPLETE;
                completeMsec = Platform::getMsec();
                audio.playSE(&Audio::SE::NO_8, 0.85f);
            }
        }
    }
}

int16_t SpriteAdventure::worldToScreenX(float worldX) const
{
    return static_cast<int16_t>(worldX - cameraX);
}

bool SpriteAdventure::isVisible(
    float worldX,
    int16_t width) const
{
    const float screenX = worldX - cameraX;
    return screenX + width >= 0.0f &&
           screenX < SCREEN_W;
}

void SpriteAdventure::drawTitle(Graphics& graphics)
{
    graphics.fillScreen(Graphics::rgb565(8, 22, 38));

    graphics.drawString(
        "SPRITE ADVENTURE",
        SCREEN_W / 2,
        52,
        Graphics::CYAN,
        Graphics::SIZE_25B,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawSprite(
        PLAYER_IDLE,
        152,
        98,
        SPRITE_W,
        SPRITE_H,
        SPRITE_SCALE,
        TRANSPARENT);

    graphics.drawString(
        "SPRITES + CAMERA",
        SCREEN_W / 2,
        138,
        Graphics::WHITE,
        Graphics::SIZE_18,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawString(
        "A / START : BEGIN",
        SCREEN_W / 2,
        184,
        Graphics::YELLOW,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);
}

void SpriteAdventure::drawWorld(Graphics& graphics)
{
    drawBackground(graphics);
    drawGround(graphics);
    drawDecorations(graphics);
    drawGems(graphics);
    drawPlayer(graphics);
    drawHud(graphics);
}

void SpriteAdventure::drawBackground(Graphics& graphics)
{
    graphics.fillScreen(Graphics::rgb565(18, 50, 82));

    // Distant scenery moves more slowly to create a small parallax effect.
    const int16_t farOffset =
        static_cast<int16_t>(cameraX * 0.22f) % 90;

    for (int16_t x = -90 - farOffset;
         x < SCREEN_W + 90;
         x += 90)
    {
        graphics.fillTriangle(
            x, 166,
            x + 48, 92,
            x + 96, 166,
            Graphics::rgb565(34, 82, 102));
    }

    graphics.fillCircle(
        270 - static_cast<int16_t>(cameraX * 0.08f),
        48,
        19,
        Graphics::rgb565(248, 216, 112));
}

void SpriteAdventure::drawGround(Graphics& graphics)
{
    graphics.fillRect(
        0,
        GROUND_Y,
        SCREEN_W,
        SCREEN_H - GROUND_Y,
        Graphics::rgb565(44, 92, 58));

    graphics.fillRect(
        0,
        GROUND_Y,
        SCREEN_W,
        5,
        Graphics::rgb565(105, 184, 91));

    const int16_t stripeOffset =
        static_cast<int16_t>(cameraX) % 32;

    for (int16_t x = -stripeOffset;
         x < SCREEN_W;
         x += 32)
    {
        graphics.fillRect(
            x,
            GROUND_Y + 17,
            16,
            4,
            Graphics::rgb565(37, 74, 48));
    }
}

void SpriteAdventure::drawDecorations(Graphics& graphics)
{
    constexpr float TREE_X[] = {
        90.0f, 236.0f, 390.0f, 548.0f,
        736.0f, 918.0f, 1080.0f
    };

    for (float worldX : TREE_X)
    {
        if (!isVisible(worldX, 24))
        {
            continue;
        }

        const int16_t x = worldToScreenX(worldX);

        graphics.fillRect(
            x + 9,
            GROUND_Y - 35,
            6,
            35,
            Graphics::rgb565(112, 72, 42));

        graphics.fillCircle(
            x + 12,
            GROUND_Y - 39,
            15,
            Graphics::rgb565(52, 146, 75));

        graphics.fillCircle(
            x + 4,
            GROUND_Y - 33,
            10,
            Graphics::rgb565(65, 168, 85));

        graphics.fillCircle(
            x + 20,
            GROUND_Y - 33,
            10,
            Graphics::rgb565(65, 168, 85));
    }
}

void SpriteAdventure::drawGems(Graphics& graphics)
{
    for (uint8_t i = 0; i < GEM_COUNT; ++i)
    {
        if (gems[i].collected ||
            !isVisible(gems[i].x, DRAW_W))
        {
            continue;
        }

        graphics.drawSprite(
            GEM_SPRITE,
            worldToScreenX(gems[i].x),
            static_cast<int16_t>(gems[i].y),
            SPRITE_W,
            SPRITE_H,
            SPRITE_SCALE,
            TRANSPARENT);
    }
}

void SpriteAdventure::drawPlayer(Graphics& graphics)
{
    const uint16_t* frame = PLAYER_IDLE;

    if (walking)
    {
        frame = (walkFrame == 0)
            ? PLAYER_WALK_1
            : PLAYER_WALK_2;
    }

    graphics.drawSprite(
        frame,
        worldToScreenX(playerX),
        static_cast<int16_t>(playerY),
        SPRITE_W,
        SPRITE_H,
        SPRITE_SCALE,
        TRANSPARENT,
        facingLeft,
        false);
}

void SpriteAdventure::drawHud(Graphics& graphics)
{
    graphics.fillRoundRect(
        8, 7, 142, 24, 6,
        Graphics::rgb565(7, 24, 38));

    char line[32];
    std::snprintf(
        line,
        sizeof(line),
        "GEMS %u / %u",
        static_cast<unsigned>(collectedCount),
        static_cast<unsigned>(GEM_COUNT));

    graphics.drawString(
        line,
        18,
        12,
        Graphics::WHITE,
        Graphics::SIZE_13);

    graphics.drawString(
        "D-PAD MOVE   B RESET   START TITLE",
        SCREEN_W / 2,
        210,
        Graphics::WHITE,
        Graphics::SIZE_10,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);
}
