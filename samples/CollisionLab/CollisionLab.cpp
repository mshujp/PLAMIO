#include "CollisionLab.h"

#include <cstdio>
#include <cstdlib>

using namespace PLAMIO;

const char* CollisionLab::getId() const { return "collisionlab"; }
const char* CollisionLab::getName() const { return "Collision Lab"; }
const char* CollisionLab::getMenuName() const { return "02 COLLISION LAB"; }

uint16_t CollisionLab::getLogicalScreenWidth() const { return 384; }
uint16_t CollisionLab::getLogicalScreenHeight() const { return 288; }
uint16_t CollisionLab::getTargetScreenWidth() const { return 320; }
uint16_t CollisionLab::getTargetScreenHeight() const { return 240; }

void CollisionLab::onInit(Storage& storage)
{
    (void)storage;
    mode = MODE_TITLE;
    resetRun();
    dirty = true;
}

void CollisionLab::resetRun()
{
    playerX = 38.0f;
    playerY = 120.0f;
    completedTests = 0;
    runStartMsec = Platform::getMsec();
    completedMsec = 0;
    messageActive = false;
    shakeActive = false;
    message[0] = '\0';
}

void CollisionLab::beginTest(uint8_t bit, const char* text, Audio& audio)
{
    if ((completedTests & bit) != 0) return;

    completedTests |= bit;
    std::snprintf(message, sizeof(message), "%s PASS", text);
    messageStartMsec = Platform::getMsec();
    messageActive = true;
    startShake();
    audio.playSE(&Audio::SE::NO_3, 0.65f);

    if (completedTests == TEST_ALL)
    {
        mode = MODE_COMPLETE;
        completedMsec = Platform::getMsec();
    }
}

void CollisionLab::startShake()
{
    shakeStartMsec = Platform::getMsec();
    shakeActive = true;
}

void CollisionLab::updateCollisionTests(Audio& audio)
{
    // 1) pointRect: player nose point enters scanner rectangle.
    if (Collision::pointRect(playerX, playerY, 76.0f, 43.0f, 50.0f, 34.0f))
    {
        beginTest(TEST_POINT_RECT, "PointRect", audio);
    }

    // 2) rectRect: square hitbox around player overlaps the cargo block.
    const float playerRectX = playerX - 7.0f;
    const float playerRectY = playerY - 7.0f;
    if (Collision::rectRect(playerRectX, playerRectY, 14.0f, 14.0f,
                            178.0f, 42.0f, 44.0f, 36.0f))
    {
        beginTest(TEST_RECT_RECT, "RectRect", audio);
    }

    // 3) circleCircle: player touches the bubble core.
    if (Collision::circleCircle(playerX, playerY, PLAYER_RADIUS,
                                104.0f, 178.0f, 17.0f))
    {
        beginTest(TEST_CIRCLE_CIRCLE, "CircleCircle", audio);
    }

    // 4) circleRect: player circle touches the pressure gate.
    if (Collision::circleRect(playerX, playerY, PLAYER_RADIUS,
                              242.0f, 151.0f, 48.0f, 30.0f))
    {
        beginTest(TEST_CIRCLE_RECT, "CircleRect", audio);
    }
}

Game::GameState CollisionLab::onUpdate(Input& input, Audio& audio,
                                        Storage& storage, float deltaSec)
{
    (void)storage;
    const uint32_t now = Platform::getMsec();

    if (messageActive && Platform::elapsed(now, messageStartMsec, MESSAGE_MSEC))
    {
        messageActive = false;
        dirty = true;
    }

    if (shakeActive && Platform::elapsed(now, shakeStartMsec, SHAKE_MSEC))
    {
        shakeActive = false;
        dirty = true;
    }

    if (mode == MODE_TITLE)
    {
        if (input.justPressed(Input::A) || input.justPressed(Input::START))
        {
            resetRun();
            mode = MODE_PLAYING;
            audio.playSE(&Audio::SE::NO_8, 0.6f);
            dirty = true;
        }
        return Game::RUNNING;
    }

    if (mode == MODE_COMPLETE)
    {
        if (Platform::elapsed(now, completedMsec, COMPLETE_WAIT_MSEC) &&
            (input.justPressed(Input::A) || input.justPressed(Input::START)))
        {
            resetRun();
            mode = MODE_PLAYING;
            audio.playSE(&Audio::SE::NO_8, 0.6f);
            dirty = true;
        }
        return Game::RUNNING;
    }

    float dx = 0.0f;
    float dy = 0.0f;
    if (input.pressed(Input::LEFT))  dx -= 1.0f;
    if (input.pressed(Input::RIGHT)) dx += 1.0f;
    if (input.pressed(Input::UP))    dy -= 1.0f;
    if (input.pressed(Input::DOWN))  dy += 1.0f;

    if (dx != 0.0f || dy != 0.0f)
    {
        Math::normalize(dx, dy);
        playerX += dx * PLAYER_SPEED * deltaSec;
        playerY += dy * PLAYER_SPEED * deltaSec;
        playerX = Math::clamp(playerX, PLAYER_RADIUS, SCREEN_W - PLAYER_RADIUS);
        playerY = Math::clamp(playerY, 30.0f + PLAYER_RADIUS,
                              SCREEN_H - 16.0f - PLAYER_RADIUS);
        updateCollisionTests(audio);
        dirty = true;
    }

    // SELECT restarts the current run.
    if (input.justPressed(Input::SELECT))
    {
        resetRun();
        audio.playSE(&Audio::SE::NO_2, 0.5f);
        dirty = true;
    }

    return Game::RUNNING;
}

void CollisionLab::drawBackground(Graphics& g)
{
    const Graphics::Color deep = Graphics::rgb565(8, 28, 45);
    const Graphics::Color grid = Graphics::rgb565(18, 55, 72);
    g.fillScreen(deep);

    for (int16_t x = 0; x < 384; x += 24)
        g.drawLine(x, 0, x, 287, grid);
    for (int16_t y = 0; y < 288; y += 24)
        g.drawLine(0, y, 383, y, grid);

    g.fillRect(0, 0, 320, 29, Graphics::rgb565(5, 18, 30));
    g.drawLine(0, 29, 319, 29, Graphics::CYAN);
}

void CollisionLab::drawStations(Graphics& g)
{
    const Graphics::Color idle = Graphics::rgb565(45, 85, 95);
    const Graphics::Color pass = Graphics::rgb565(50, 210, 150);

    Graphics::Color c = (completedTests & TEST_POINT_RECT) ? pass : idle;
    g.drawRect(76, 43, 50, 34, 2, c);
    g.drawString("PointRect", 101, 82, c, Graphics::SIZE_13,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);

    c = (completedTests & TEST_RECT_RECT) ? pass : idle;
    g.fillRect(178, 42, 44, 36, c);
    g.drawString("RectRect", 200, 82, c, Graphics::SIZE_13,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);

    c = (completedTests & TEST_CIRCLE_CIRCLE) ? pass : idle;
    g.drawCircle(104, 178, 17, c);
    g.drawCircle(104, 178, 12, c);
    g.drawString("CircleCircle", 104, 205, c, Graphics::SIZE_13,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);

    c = (completedTests & TEST_CIRCLE_RECT) ? pass : idle;
    g.drawRect(242, 151, 48, 30, 2, c);
    g.drawString("CircleRect", 266, 190, c, Graphics::SIZE_13,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
}

void CollisionLab::drawHud(Graphics& g)
{
    char buf[64];
    const uint32_t now = Platform::getMsec();
    const uint32_t runMsec = (mode == MODE_COMPLETE) ?
                             (completedMsec - runStartMsec) : (now - runStartMsec);

    std::snprintf(buf, sizeof(buf), "TEST %u/4  TIME %lu.%02lu",
                  static_cast<unsigned>(
                      ((completedTests & TEST_POINT_RECT) ? 1 : 0) +
                      ((completedTests & TEST_RECT_RECT) ? 1 : 0) +
                      ((completedTests & TEST_CIRCLE_CIRCLE) ? 1 : 0) +
                      ((completedTests & TEST_CIRCLE_RECT) ? 1 : 0)),
                  static_cast<unsigned long>(runMsec / 1000),
                  static_cast<unsigned long>((runMsec % 1000) / 10));
    g.drawString(buf, 8, 15, Graphics::WHITE, Graphics::SIZE_13,
                 Graphics::HorizontalAlign::LEFT, Graphics::VerticalAlign::MIDDLE);

    if (messageActive)
    {
        g.fillRoundRect(74, 97, 172, 35, 7, Graphics::rgb565(2, 15, 25));
        g.drawRoundRect(74, 97, 172, 35, 7, Graphics::CYAN);
        g.drawString(message, 160, 114, Graphics::WHITE, Graphics::SIZE_18,
                     Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
    }

    g.fillCircle(static_cast<int16_t>(playerX), static_cast<int16_t>(playerY),
                 static_cast<uint16_t>(PLAYER_RADIUS), Graphics::YELLOW);
    // The collision test still uses one exact point. The cross only makes it visible.
    const int16_t markerX = static_cast<int16_t>(playerX);
    const int16_t markerY = static_cast<int16_t>(playerY);
    g.drawLine(markerX - 3, markerY, markerX + 3, markerY, Graphics::RED);
    g.drawLine(markerX, markerY - 3, markerX, markerY + 3, Graphics::RED);
}

void CollisionLab::drawTitle(Graphics& g)
{
    g.drawString("COLLISION LAB", 160, 62, Graphics::CYAN, Graphics::SIZE_32B,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
    g.drawString("COLLISION API SAMPLE", 160, 93, Graphics::LIGHTGRAY, Graphics::SIZE_18,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);

    g.drawString("D-PAD : MOVE", 160, 132, Graphics::WHITE, Graphics::SIZE_18,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
    g.drawString("TOUCH ALL 4 TARGETS", 160, 155, Graphics::WHITE, Graphics::SIZE_18,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
    g.drawString("A / START", 160, 190, Graphics::YELLOW, Graphics::SIZE_22B,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
}

void CollisionLab::drawComplete(Graphics& g)
{
    char buf[64];
    const uint32_t result = completedMsec - runStartMsec;

    g.fillRoundRect(45, 68, 230, 112, 12, Graphics::rgb565(3, 18, 28));
    g.drawRoundRect(45, 68, 230, 112, 12, 2, Graphics::GREEN);
    g.drawString("ALL COLLISIONS PASS", 160, 93, Graphics::GREEN, Graphics::SIZE_25B,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);

    std::snprintf(buf, sizeof(buf), "TIME %lu.%02lu SEC",
                  static_cast<unsigned long>(result / 1000),
                  static_cast<unsigned long>((result % 1000) / 10));
    g.drawString(buf, 160, 127, Graphics::WHITE, Graphics::SIZE_18,
                 Graphics::HorizontalAlign::CENTER, Graphics::VerticalAlign::MIDDLE);
    g.drawString("A / START : RETRY", 160, 157, Graphics::YELLOW,
                 Graphics::SIZE_13, Graphics::HorizontalAlign::CENTER,
                 Graphics::VerticalAlign::MIDDLE);
}

bool CollisionLab::onDraw(Graphics& g, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty) return false;

    // Screen shake is a visual effect only; collision coordinates do not change.
    if (shakeActive)
    {
        const int16_t shakeX = static_cast<int16_t>((std::rand() % 13) - 6);
        const int16_t shakeY = static_cast<int16_t>((std::rand() % 13) - 6);
        g.setViewport(shakeX, shakeY);
        }
    else
    {
        g.resetViewport();
        }

    drawBackground(g);

    if (mode == MODE_TITLE)
    {
        drawTitle(g);
    }
    else
    {
        drawStations(g);
        drawHud(g);
        if (mode == MODE_COMPLETE) drawComplete(g);
    }

    dirty = false;
    return true;
}

void CollisionLab::onTerminate(Storage& storage)
{
    (void)storage;
}
