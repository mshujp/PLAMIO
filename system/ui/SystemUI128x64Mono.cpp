#include "SystemUI128x64Mono.h"

#include <stdio.h>

using namespace PLAMIO;

namespace
{
    constexpr uint16_t SCREEN_W = SystemUI128x64Mono::SCREEEN_WIDTH;
    constexpr uint16_t SCREEN_H = SystemUI128x64Mono::SCREEEN_HIGHT;

    Graphics::Color COL_BG;
    Graphics::Color COL_FG;

    void drawHeader(Graphics& g, const char* title)
    {
        g.fillScreen(COL_BG);
        g.drawString(title, 0, 0, COL_FG, Graphics::Font::SIZE_10);
        g.drawLine(0, 12, SCREEN_W - 1, 12, COL_FG);
    }

    void drawPageText(Graphics& g, uint16_t page, uint16_t pageCount)
    {
        char buf[12];
        snprintf(buf, sizeof(buf), "%u/%u", static_cast<unsigned>(page + 1), static_cast<unsigned>(pageCount));
        g.drawString(buf, SCREEN_W - 1, 0, COL_FG, Graphics::Font::SIZE_10, Graphics::HorizontalAlign::RIGHT, Graphics::VerticalAlign::TOP);
    }

    void drawSlot(Graphics& g, int y, const char* text, bool selected)
    {
        if (selected)
        {
            g.fillRect(0, y, SCREEN_W, 18, COL_FG);
            g.drawString(text, 8, y + 4, COL_BG, Graphics::Font::SIZE_10);
        }
        else
        {
            g.drawRect(0, y, SCREEN_W, 18, COL_FG);
            g.drawString(text, 8, y + 4, COL_FG, Graphics::Font::SIZE_10);
        }
    }
}

SystemUI128x64Mono::SystemUI128x64Mono(Graphics::Color onColor, Graphics::Color offColor)
{
    COL_BG = offColor;
    COL_FG = onColor;
}

void SystemUI128x64Mono::drawSplash(Graphics& graphics)
{
    graphics.fillScreen(COL_BG);

    graphics.drawString(
        "PLAMIO",
        SCREEN_W / 2,
        20,
        COL_FG,
        Graphics::Font::SIZE_18,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    uint16_t barW = static_cast<uint16_t>((static_cast<uint32_t>(splashFrames) * 80u) / 24u);
    if (barW > 80) barW = 80;

    graphics.drawRect(24, 46, 80, 6, COL_FG);
    graphics.fillRect(24, 46, barW, 6, COL_FG);
}

void SystemUI128x64Mono::drawSelect(Graphics& graphics)
{
    drawHeader(graphics, getCurrentMenuTitle());
    drawPageText(graphics, pageIndex, getPageCount());

    constexpr int startY = 18;
    constexpr int gap = 6;
    constexpr int slotH = 18;

    for (uint16_t i = 0; i < ITEMS_PER_PAGE; i++)
    {
        const uint16_t index = pageIndex * ITEMS_PER_PAGE + i;
        const int y = startY + i * (slotH + gap);

        drawSlot(
            graphics,
            y,
            getSlotName(index),
            index == selectedIndex);
    }
}

void SystemUI128x64Mono::drawInfo(Graphics& graphics)
{
    SystemUI::SystemInfo info = {};
    if (getSystemInfoHandler != nullptr)
    {
        getSystemInfoHandler(info, getSystemInfoContext);
    }

    drawHeader(graphics, "INFO");

    graphics.drawString("VER", 0, 16, COL_FG, Graphics::Font::SIZE_10);
    graphics.drawString(info.version, 36, 16, COL_FG, Graphics::Font::SIZE_10);

    graphics.drawString("DISP", 0, 28, COL_FG, Graphics::Font::SIZE_10);
    graphics.drawString(info.display, 36, 28, COL_FG, Graphics::Font::SIZE_10);

    graphics.drawString("BAT", 0, 40, COL_FG, Graphics::Font::SIZE_10);
    graphics.drawString(info.battery, 36, 40, COL_FG, Graphics::Font::SIZE_10);

    graphics.drawString("INPUT", 0, 52, COL_FG, Graphics::Font::SIZE_10);
    graphics.drawString(info.input, 36, 53, COL_FG, Graphics::Font::SIZE_10);

}

void SystemUI128x64Mono::drawShutdownConfirm(Graphics& graphics)
{
    graphics.fillScreen(COL_BG);

    graphics.drawString(
        "SHUT DOWN?",
        SCREEN_W / 2,
        4,
        COL_FG,
        Graphics::Font::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawLine(8, 21, SCREEN_W - 9, 21, COL_FG);

    constexpr int buttonY = 32;
    constexpr int buttonW = 48;
    constexpr int buttonH = 22;
    constexpr int yesX = 10;
    constexpr int noX = 70;

    if (shutdownYesSelected)
    {
        graphics.fillRect(yesX, buttonY, buttonW, buttonH, COL_FG);
        graphics.drawString(
            "YES",
            yesX + buttonW / 2,
            buttonY + buttonH / 2,
            COL_BG,
            Graphics::Font::SIZE_13,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::MIDDLE);

        graphics.drawRect(noX, buttonY, buttonW, buttonH, COL_FG);
        graphics.drawString(
            "NO",
            noX + buttonW / 2,
            buttonY + buttonH / 2,
            COL_FG,
            Graphics::Font::SIZE_13,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::MIDDLE);
    }
    else
    {
        graphics.drawRect(yesX, buttonY, buttonW, buttonH, COL_FG);
        graphics.drawString(
            "YES",
            yesX + buttonW / 2,
            buttonY + buttonH / 2,
            COL_FG,
            Graphics::Font::SIZE_13,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::MIDDLE);

        graphics.fillRect(noX, buttonY, buttonW, buttonH, COL_FG);
        graphics.drawString(
            "NO",
            noX + buttonW / 2,
            buttonY + buttonH / 2,
            COL_BG,
            Graphics::Font::SIZE_13,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::MIDDLE);
    }
}

void SystemUI128x64Mono::drawPowerOffReady(Graphics& graphics)
{
    graphics.fillScreen(COL_BG);

    graphics.drawRect(4, 4, SCREEN_W - 8, SCREEN_H - 8, COL_FG);

    graphics.drawString(
        "SAFE TO",
        SCREEN_W / 2,
        13,
        COL_FG,
        Graphics::Font::SIZE_18,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawString(
        "POWER OFF",
        SCREEN_W / 2,
        34,
        COL_FG,
        Graphics::Font::SIZE_18,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);
}

void SystemUI128x64Mono::drawLowBttery(Graphics& graphics)
{
    graphics.fillScreen(COL_BG);

    graphics.drawRect(4, 4, SCREEN_W - 8, SCREEN_H - 8, COL_FG);

    graphics.drawString(
        "SAFE TO",
        SCREEN_W / 2,
        SCREEN_H / 2,
        COL_FG,
        Graphics::Font::SIZE_18,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::MIDDLE);
}
