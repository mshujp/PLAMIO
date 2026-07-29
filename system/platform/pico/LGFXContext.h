#pragma once

#include "InputTouchConfig.h"
#include "LGFXTouchXPT2046.h"
#include "PRUZEA.h"
#include <LovyanGFX.hpp>
#include <stdint.h>

namespace PRUZEA {

class LGFXContext : public lgfx::LGFX_Device
{
protected:
    lgfx::Panel_ILI9341 panel;

    LGFXContext() = default;

    void configurePanel(lgfx::IBus& bus, int8_t csPin, int8_t resetPin, bool busShared);
    void configureTouch(const InputTouchConfig& config, bool busShared);
    void finishConfiguration();

private:
    LGFXTouchXPT2046 touch;
    bool touchConfigured = false;
    bool touchFinalized = false;
    int16_t minimumTouchPressure = 0;

public:
    virtual ~LGFXContext() = default;
    virtual void enableTouch(const InputTouchConfig& config) = 0;

    void finalizeTouchInitialization();
    bool isTouchAvailable() const;
    bool readTouch(int16_t& x, int16_t& y);
};

} // namespace PRUZEA
