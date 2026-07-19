#pragma once

#include "SystemUI.h"

namespace PLAMIO
{

class SystemUI128x64Mono : public SystemUI
{
private:
    static constexpr uint16_t ITEMS_PER_PAGE = 2;

protected:
    uint16_t getItemsPerPage() const override { return ITEMS_PER_PAGE; }

    void drawSplash(Graphics& graphics) override;
    void drawSelect(Graphics& graphics) override;
    void drawInfo(Graphics& graphics) override;
    void drawShutdownConfirm(Graphics& graphics) override;

public:
    static constexpr uint16_t SCREEEN_WIDTH = 128;
    static constexpr uint16_t SCREEEN_HIGHT = 64;

    SystemUI128x64Mono(Graphics::Color onColor = Graphics::Color::WHITE, Graphics::Color offColor = Graphics::Color::BLACK);

    uint16_t getLogicalScreenWidth() const override { return SystemUI128x64Mono::SCREEEN_WIDTH; }
    uint16_t getLogicalScreenHeight() const override { return SystemUI128x64Mono::SCREEEN_HIGHT; }
    uint16_t getTargetScreenWidth() const override { return getLogicalScreenWidth(); }
    uint16_t getTargetScreenHeight() const override { return getLogicalScreenHeight(); }
    void drawPowerOffReady(Graphics& graphics) override;
    void drawLowBttery(Graphics& graphics) override;
};

} // namespace PLAMIO
