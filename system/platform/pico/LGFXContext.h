#pragma once

#include "PRUZEA.h"
#include <LovyanGFX.hpp>
#include <algorithm>
#include <cstring>
#include <hardware/spi.h>
#include <stdint.h>

namespace PRUZEA {

struct InputTouchConfig
{
    uint8_t spiHost = 0;
    uint32_t spiFreq = 2000000;
    int8_t clkPin = -1;
    int8_t mosiPin = -1;
    int8_t misoPin = -1;
    int8_t csPin = -1;
    int8_t irqPin = -1;

    int16_t minX = 250;
    int16_t maxX = 3850;
    int16_t minY = 250;
    int16_t maxY = 3850;
    int16_t minZ = 2048;

    uint16_t nativeWidth = 240;
    uint16_t nativeHeight = 320;
    uint8_t offsetRotation = 0;
};

class LGFXTouchXPT2046 : public lgfx::Touch_XPT2046
{
public:
    uint_fast8_t getTouchRaw(lgfx::touch_point_t* point, uint_fast8_t count) override
    {
        if (!_inited || point == nullptr || count == 0 || _cfg.spi_host < 0) return 0;

        point->size = 0;
        if (_cfg.pin_int >= 0 && lgfx::gpio_in(_cfg.pin_int)) return 0;

        uint8_t transmitData[59]{};
        uint8_t receiveData[59]{};
        transmitData[0] = 0x91;
        transmitData[2] = 0xB1;
        transmitData[4] = 0xD1;
        transmitData[6] = 0xC1;
        transmitData[56] = 0x80;
        std::memcpy(&transmitData[8], transmitData, 8);
        std::memcpy(&transmitData[16], transmitData, 16);
        std::memcpy(&transmitData[32], transmitData, 24);
        // Complete the final power-down command before releasing CS so that
        // XPT2046 enters PENIRQ mode.
        transmitData[57] = 0;
        transmitData[58] = 0;

        lgfx::spi::beginTransaction(_cfg.spi_host, _cfg.freq, 0);
        if (_cfg.pin_cs >= 0) lgfx::gpio_lo(_cfg.pin_cs);

        spi_inst_t* spi = _cfg.spi_host == 0 ? spi0 : spi1;
        spi_write_read_blocking(spi, transmitData, receiveData, sizeof(transmitData));

        if (_cfg.pin_cs >= 0) lgfx::gpio_hi(_cfg.pin_cs);
        lgfx::spi::endTransaction(_cfg.spi_host);

        size_t xCount = 0;
        size_t yCount = 0;
        size_t zCount = 0;
        uint_fast16_t xValues[7];
        uint_fast16_t yValues[7];
        uint_fast16_t zValues[7];

        for (size_t i = 0; i < 7; ++i)
        {
            const uint8_t* data = &receiveData[i * 8];
            const int32_t x = (data[5] << 8 | data[6]) >> 3;
            const int32_t y = (data[1] << 8 | data[2]) >> 3;
            const int32_t z =
                0x3200 + y - x +
                (((data[3] << 8 | data[4]) -
                  (data[7] << 8 | data[8])) >> 1);

            if (x > 128 && x <= 3968) xValues[xCount++] = x;
            if (y > 128 && y <= 3968) yValues[yCount++] = y;
            if (z > 0) zValues[zCount++] = z;
        }

        if (xCount < 3 || yCount < 3 || zCount < 3) return 0;

        std::sort(xValues, xValues + xCount);
        std::sort(yValues, yValues + yCount);
        std::sort(zValues, zValues + zCount);

        point->x = xValues[xCount >> 1];
        point->y = yValues[yCount >> 1];
        point->size = zValues[zCount >> 1] >> 8;
        return point->size ? 1 : 0;
    }
};

class LGFXContext : public lgfx::LGFX_Device
{
private:
    lgfx::Panel_ILI9341 panel;
    lgfx::Bus_SPI bus;
    LGFXTouchXPT2046 touch;
    bool touchConfigured = false;
    bool touchFinalized = false;
    int16_t minimumTouchPressure = 0;

public:
    LGFXContext(
        uint8_t spiHost,
        uint32_t spiWriteFreq,
        int8_t clkPin,
        int8_t dataPin,
        int8_t dcPin,
        int8_t csPin,
        int8_t resetPin,
        const InputTouchConfig* touchConfig = nullptr)
    {
        auto busConfig = bus.config();
        busConfig.spi_host = static_cast<decltype(busConfig.spi_host)>(spiHost);
        busConfig.spi_mode = 0;
        busConfig.freq_write = spiWriteFreq;
        busConfig.freq_read = 16000000;
        busConfig.pin_sclk = clkPin;
        busConfig.pin_mosi = dataPin;
        busConfig.pin_miso =
            touchConfig != nullptr && touchConfig->spiHost == spiHost
                ? touchConfig->misoPin
                : -1;
        busConfig.pin_dc = dcPin;
        bus.config(busConfig);
        panel.setBus(&bus);

        auto panelConfig = panel.config();
        panelConfig.pin_cs = csPin;
        panelConfig.pin_rst = resetPin;
        panelConfig.pin_busy = -1;
        panelConfig.panel_width = Display::ILI9341_SCREEN_H;
        panelConfig.panel_height = Display::ILI9341_SCREEN_W;
        panelConfig.memory_width = Display::ILI9341_SCREEN_H;
        panelConfig.memory_height = Display::ILI9341_SCREEN_W;
        panelConfig.readable = false;
        panelConfig.invert = false;
        panelConfig.rgb_order = false;
        panelConfig.dlen_16bit = false;
        panelConfig.bus_shared = true;
        panel.config(panelConfig);

        if (touchConfig != nullptr)
        {
            auto touchDriverConfig = touch.config();
            touchDriverConfig.spi_host = static_cast<decltype(touchDriverConfig.spi_host)>(touchConfig->spiHost);
            touchDriverConfig.freq = touchConfig->spiFreq;
            touchDriverConfig.pin_sclk = touchConfig->clkPin;
            touchDriverConfig.pin_mosi = touchConfig->mosiPin;
            touchDriverConfig.pin_miso = touchConfig->misoPin;
            touchDriverConfig.pin_cs = touchConfig->csPin;
            // IRQ is intentionally disabled. XPT2046 is read by polling.
            // InputTouchConfig::irqPin is kept for API compatibility and future use.
            touchDriverConfig.pin_int = -1;
            touchDriverConfig.x_min = touchConfig->minX;
            touchDriverConfig.x_max = touchConfig->maxX;
            touchDriverConfig.y_min = touchConfig->minY;
            touchDriverConfig.y_max = touchConfig->maxY;
            touchDriverConfig.offset_rotation = touchConfig->offsetRotation;
            touchDriverConfig.bus_shared = touchConfig->spiHost == spiHost;
            touch.config(touchDriverConfig);
            panel.setTouch(&touch);

            touchConfigured =
                touchConfig->clkPin >= 0 &&
                touchConfig->mosiPin >= 0 &&
                touchConfig->misoPin >= 0 &&
                touchConfig->csPin >= 0;
            minimumTouchPressure = touchConfig->minZ;
        }

        setPanel(&panel);
    }

    void finalizeTouchInitialization()
    {
        if (!touchConfigured || touchFinalized) return;

        // Keep XPT2046 in polling mode.
        // InputTouchConfig::irqPin is intentionally not used internally.
        auto touchDriverConfig = touch.config();
        touchDriverConfig.pin_int = -1;
        touch.config(touchDriverConfig);

        touchFinalized = true;
    }

    bool isTouchAvailable() const
    {
        return touchConfigured;
    }

    bool readTouch(int16_t& x, int16_t& y)
    {
        if (!touchConfigured) return false;

        lgfx::touch_point_t point{};
        const uint_fast8_t touchResult = getTouch(&point);

        const bool pressureAccepted = static_cast<uint32_t>(point.size) * 256 >= static_cast<uint16_t>(minimumTouchPressure);

        if (touchResult == 0 || !pressureAccepted) return false;

        x = point.x;
        y = point.y;
        return true;
    }

};

} // namespace PRUZEA
