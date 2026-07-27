#pragma once
#include "PRUZEA.h"

class TouchPaint : public PRUZEA::Game
{
public:
    const char* getId() const override { return "touch_paint"; }
    const char* getName() const override { return "Touch Paint"; }
    const char* getMenuName() const override { return "12 Touch Paint"; }
    const char* getMenuGroup() const override { return "SAMPLES"; }

    uint16_t getLogicalScreenWidth() const override { return 320; }
    uint16_t getLogicalScreenHeight() const override { return 240; }
    uint16_t getTargetScreenWidth() const override { return 320; }
    uint16_t getTargetScreenHeight() const override { return 240; }

protected:
    void onInit(PRUZEA::Storage& storage) override;
    GameState onUpdate(
        PRUZEA::Input& input,
        PRUZEA::Audio& audio,
        PRUZEA::Storage& storage,
        float deltaSec) override;
    bool onDraw(
        PRUZEA::Graphics& graphics,
        bool requestFullRedraw) override;
    void onTerminate(PRUZEA::Storage& storage) override;

private:
    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;

    static constexpr int16_t CANVAS_X = 8;
    static constexpr int16_t CANVAS_Y = 36;
    static constexpr int16_t CELL_SIZE = 4;
    static constexpr int16_t GRID_W = 76;
    static constexpr int16_t GRID_H = 39;
    static constexpr int16_t CANVAS_W = GRID_W * CELL_SIZE;
    static constexpr int16_t CANVAS_H = GRID_H * CELL_SIZE;

    static constexpr int16_t TOOLBAR_Y = 200;
    static constexpr int16_t TOOLBAR_H = 40;
    static constexpr int16_t TOOL_W = 64;

    enum PaintColor : uint8_t
    {
        COLOR_NONE = 0,
        COLOR_YELLOW,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_WHITE
    };

    PaintColor selectedColor = COLOR_YELLOW;
    uint8_t pixels[GRID_H][GRID_W] = {};

    static bool inside(
        int16_t px,
        int16_t py,
        int16_t x,
        int16_t y,
        int16_t w,
        int16_t h);
    static PRUZEA::Graphics::Color getGraphicsColor(PaintColor color);

    void clearCanvas();
    void paintAt(int16_t screenX, int16_t screenY);
    bool selectTool(int16_t screenX, int16_t screenY);
    void drawCanvas(PRUZEA::Graphics& graphics);
    void drawToolbar(PRUZEA::Graphics& graphics);
    void drawColorButton(
        PRUZEA::Graphics& graphics,
        PaintColor color,
        int16_t x);
};
