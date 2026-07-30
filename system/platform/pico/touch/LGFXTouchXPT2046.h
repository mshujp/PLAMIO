#pragma once

#include <LovyanGFX.hpp>
#include <algorithm>
#include <cstring>
#include <hardware/spi.h>

namespace PRUZEA {

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

} // namespace PRUZEA
