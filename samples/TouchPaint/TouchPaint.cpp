#include "TouchPaint.h"
#include <cstring>

using namespace PRUZEA;

void TouchPaint::onInit(Storage& storage)
{
    (void)storage;

    selectedColor = COLOR_YELLOW;
    clearCanvas();
    dirty = true;
}

Game::GameState TouchPaint::onUpdate(
    Input& input,
    Audio& audio,
    Storage& storage,
    float deltaSec)
{
    (void)storage;
    (void)deltaSec;

    if (input.justPressed(Input::START))
    {
        return GameState::TERMINATE_REQUEST;
    }

    if (!input.touched())
    {
        return GameState::RUNNING;
    }

    const int16_t x = input.touchX();
    const int16_t y = input.touchY();
    if (x < 0 || y < 0)
    {
        return GameState::RUNNING;
    }

    if (inside(x, y, CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H))
    {
        paintAt(x, y);
        dirty = true;
    }
    else if (input.justTouched() && selectTool(x, y))
    {
        audio.playSE(&Audio::SE::NO_1, 0.35f);
        dirty = true;
    }

    return GameState::RUNNING;
}

bool TouchPaint::onDraw(Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty)
    {
        return false;
    }

    graphics.fillScreen(Graphics::BLACK);

    graphics.drawString(
        "TOUCH PAINT",
        SCREEN_W / 2,
        6,
        Graphics::WHITE,
        Graphics::SIZE_22B,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    drawCanvas(graphics);
    drawToolbar(graphics);

    dirty = false;
    return true;
}

void TouchPaint::onTerminate(Storage& storage)
{
    (void)storage;
}

bool TouchPaint::inside(
    int16_t px,
    int16_t py,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h)
{
    return px >= x && px < x + w && py >= y && py < y + h;
}

Graphics::Color TouchPaint::getGraphicsColor(PaintColor color)
{
    switch (color)
    {
        case COLOR_YELLOW:  return Graphics::YELLOW;
        case COLOR_CYAN:    return Graphics::CYAN;
        case COLOR_MAGENTA: return Graphics::MAGENTA;
        case COLOR_WHITE:   return Graphics::WHITE;
        default:            return Graphics::BLACK;
    }
}

void TouchPaint::clearCanvas()
{
    std::memset(pixels, 0, sizeof(pixels));
}

void TouchPaint::paintAt(int16_t screenX, int16_t screenY)
{
    const int16_t gridX = (screenX - CANVAS_X) / CELL_SIZE;
    const int16_t gridY = (screenY - CANVAS_Y) / CELL_SIZE;

    // A 3 x 3 cell brush is easy to use with a fingertip or resistive stylus.
    for (int16_t oy = -1; oy <= 1; ++oy)
    {
        for (int16_t ox = -1; ox <= 1; ++ox)
        {
            const int16_t x = gridX + ox;
            const int16_t y = gridY + oy;
            if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H)
            {
                pixels[y][x] = static_cast<uint8_t>(selectedColor);
            }
        }
    }
}

bool TouchPaint::selectTool(int16_t screenX, int16_t screenY)
{
    if (!inside(screenX, screenY, 0, TOOLBAR_Y, SCREEN_W, TOOLBAR_H))
    {
        return false;
    }

    const int16_t toolIndex = screenX / TOOL_W;
    switch (toolIndex)
    {
        case 0: selectedColor = COLOR_YELLOW;  break;
        case 1: selectedColor = COLOR_CYAN;    break;
        case 2: selectedColor = COLOR_MAGENTA; break;
        case 3: selectedColor = COLOR_WHITE;   break;
        case 4: clearCanvas();                 break;
        default: return false;
    }

    return true;
}

void TouchPaint::drawCanvas(Graphics& graphics)
{
    graphics.fillRect(
        CANVAS_X - 2,
        CANVAS_Y - 2,
        CANVAS_W + 4,
        CANVAS_H + 4,
        Graphics::DARKGRAY);
    graphics.fillRect(
        CANVAS_X,
        CANVAS_Y,
        CANVAS_W,
        CANVAS_H,
        Graphics::BLACK);

    for (int16_t y = 0; y < GRID_H; ++y)
    {
        for (int16_t x = 0; x < GRID_W; ++x)
        {
            const PaintColor color = static_cast<PaintColor>(pixels[y][x]);
            if (color == COLOR_NONE)
            {
                continue;
            }

            graphics.fillRect(
                CANVAS_X + x * CELL_SIZE,
                CANVAS_Y + y * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE,
                getGraphicsColor(color));
        }
    }
}

void TouchPaint::drawColorButton(
    Graphics& graphics,
    PaintColor color,
    int16_t x)
{
    const bool selected = selectedColor == color;
    const Graphics::Color fillColor = getGraphicsColor(color);

    graphics.fillRect(x + 3, TOOLBAR_Y + 4, TOOL_W - 6, TOOLBAR_H - 8, fillColor);
    graphics.drawRect(
        x + 2,
        TOOLBAR_Y + 3,
        TOOL_W - 4,
        TOOLBAR_H - 6,
        selected ? 3 : 1,
        selected ? Graphics::ORANGE : Graphics::DARKGRAY);
}

void TouchPaint::drawToolbar(Graphics& graphics)
{
    drawColorButton(graphics, COLOR_YELLOW, 0 * TOOL_W);
    drawColorButton(graphics, COLOR_CYAN, 1 * TOOL_W);
    drawColorButton(graphics, COLOR_MAGENTA, 2 * TOOL_W);
    drawColorButton(graphics, COLOR_WHITE, 3 * TOOL_W);

    const int16_t clearX = 4 * TOOL_W;
    graphics.fillRect(
        clearX + 3,
        TOOLBAR_Y + 4,
        TOOL_W - 6,
        TOOLBAR_H - 8,
        Graphics::DARKGRAY);
    graphics.drawRect(
        clearX + 2,
        TOOLBAR_Y + 3,
        TOOL_W - 4,
        TOOLBAR_H - 6,
        Graphics::LIGHTGRAY);
    graphics.drawString(
        "CLEAR",
        clearX + TOOL_W / 2,
        TOOLBAR_Y + TOOLBAR_H / 2,
        Graphics::WHITE,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::MIDDLE);
}
