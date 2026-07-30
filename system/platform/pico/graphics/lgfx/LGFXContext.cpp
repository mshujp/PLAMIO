#include "LGFXContext.h"

using namespace PRUZEA;

void LGFXContext::configurePanel(lgfx::IBus& bus, int8_t csPin, int8_t resetPin, bool busShared)
{
    panel.setBus(&bus);

    auto config = panel.config();
    config.pin_cs = csPin;
    config.pin_rst = resetPin;
    config.pin_busy = -1;
    config.panel_width = Display::ILI9341_SCREEN_H;
    config.panel_height = Display::ILI9341_SCREEN_W;
    config.memory_width = Display::ILI9341_SCREEN_H;
    config.memory_height = Display::ILI9341_SCREEN_W;
    config.readable = false;
    config.invert = false;
    config.rgb_order = false;
    config.dlen_16bit = false;
    config.bus_shared = busShared;
    panel.config(config);
}

void LGFXContext::configureTouch(const InputTouchConfig& config, bool busShared)
{
    auto driverConfig = touch.config();
    driverConfig.spi_host = static_cast<decltype(driverConfig.spi_host)>(config.spiHost);
    driverConfig.freq = config.spiFreq;
    driverConfig.pin_sclk = config.clkPin;
    driverConfig.pin_mosi = config.mosiPin;
    driverConfig.pin_miso = config.misoPin;
    driverConfig.pin_cs = config.csPin;
    driverConfig.pin_int = -1;
    driverConfig.x_min = config.minX;
    driverConfig.x_max = config.maxX;
    driverConfig.y_min = config.minY;
    driverConfig.y_max = config.maxY;
    driverConfig.offset_rotation = config.offsetRotation;
    driverConfig.bus_shared = busShared;
    touch.config(driverConfig);
    panel.setTouch(&touch);

    touchConfigured =
        config.clkPin >= 0 &&
        config.mosiPin >= 0 &&
        config.misoPin >= 0 &&
        config.csPin >= 0;
    minimumTouchPressure = config.minZ;
}

void LGFXContext::finishConfiguration()
{
    setPanel(&panel);
}

void LGFXContext::finalizeTouchInitialization()
{
    if (!touchConfigured || touchFinalized) return;

    auto config = touch.config();
    config.pin_int = -1;
    touch.config(config);
    touchFinalized = true;
}

bool LGFXContext::isTouchAvailable() const
{
    return touchConfigured;
}

bool LGFXContext::readTouch(int16_t& x, int16_t& y)
{
    if (!touchConfigured) return false;

    lgfx::touch_point_t point{};
    const uint_fast8_t result = getTouch(&point);
    const bool pressureAccepted =
        static_cast<uint32_t>(point.size) * 256 >=
        static_cast<uint16_t>(minimumTouchPressure);
    if (result == 0 || !pressureAccepted) return false;

    x = point.x;
    y = point.y;
    return true;
}
