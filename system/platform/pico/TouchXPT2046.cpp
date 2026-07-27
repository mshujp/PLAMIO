/*
 * XPT2046 transfer sequence derived from XPT2046_Touchscreen.
 * Copyright (c) 2015 Paul Stoffregen, paul@pjrc.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TouchXPT2046.h"
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>

using namespace PRUZEA;

namespace
{

constexpr uint8_t COMMAND_Z1 = 0xB1;
constexpr uint8_t COMMAND_Z2 = 0xC1;
constexpr uint8_t COMMAND_X = 0x91;
constexpr uint8_t COMMAND_Y = 0xD1;
constexpr uint8_t COMMAND_Y_POWER_DOWN = 0xD0;
constexpr uint16_t ADC_MAX = 4095;

}

TouchXPT2046::TouchXPT2046(const InputTouchConfig& touchConfig)
    : config(touchConfig)
{
}

bool TouchXPT2046::begin()
{
    end();

    if (config.spiHost > 1 || config.spiFreq == 0 ||
        config.clkPin < 0 || config.mosiPin < 0 || config.misoPin < 0 || config.csPin < 0 ||
        config.nativeWidth == 0 || config.nativeHeight == 0)
    {
        return false;
    }

    spi = config.spiHost == 0 ? spi0 : spi1;
    spi_init(spi, config.spiFreq);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(static_cast<uint>(config.clkPin), GPIO_FUNC_SPI);
    gpio_set_function(static_cast<uint>(config.mosiPin), GPIO_FUNC_SPI);
    gpio_set_function(static_cast<uint>(config.misoPin), GPIO_FUNC_SPI);

    gpio_init(static_cast<uint>(config.csPin));
    gpio_set_dir(static_cast<uint>(config.csPin), GPIO_OUT);
    gpio_put(static_cast<uint>(config.csPin), 1);

    if (config.irqPin >= 0)
    {
        gpio_init(static_cast<uint>(config.irqPin));
        gpio_set_dir(static_cast<uint>(config.irqPin), GPIO_IN);
    }

    available = true;
    return true;
}

void TouchXPT2046::end()
{
    if (available && config.csPin >= 0)
    {
        gpio_put(static_cast<uint>(config.csPin), 1);
    }

    available = false;
    currentTouched = false;
    previousTouched = false;
    currentX = -1;
    currentY = -1;
    spi = nullptr;
}

uint8_t TouchXPT2046::transferByte(uint8_t data)
{
    uint8_t received = 0;
    spi_write_read_blocking(spi, &data, &received, 1);
    return received;
}

uint16_t TouchXPT2046::transfer16(uint16_t data)
{
    const uint8_t tx[] = {
        static_cast<uint8_t>(data >> 8),
        static_cast<uint8_t>(data)
    };
    uint8_t rx[sizeof(tx)] = {};
    spi_write_read_blocking(spi, tx, rx, sizeof(tx));
    return static_cast<uint16_t>((static_cast<uint16_t>(rx[0]) << 8) | rx[1]);
}

int16_t TouchXPT2046::bestTwoAverage(int16_t a, int16_t b, int16_t c)
{
    const int16_t ab = a > b ? a - b : b - a;
    const int16_t ac = a > c ? a - c : c - a;
    const int16_t bc = b > c ? b - c : c - b;

    if (ab <= ac && ab <= bc) return static_cast<int16_t>((a + b) >> 1);
    if (ac <= ab && ac <= bc) return static_cast<int16_t>((a + c) >> 1);
    return static_cast<int16_t>((b + c) >> 1);
}

int16_t TouchXPT2046::mapClamped(int32_t value, int32_t inMin, int32_t inMax, int16_t outMin, int16_t outMax)
{
    if (inMin == inMax) return outMin;

    int32_t mapped = (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    const int16_t low = outMin < outMax ? outMin : outMax;
    const int16_t high = outMin < outMax ? outMax : outMin;
    if (mapped < low) mapped = low;
    if (mapped > high) mapped = high;
    return static_cast<int16_t>(mapped);
}

void TouchXPT2046::update()
{
    previousTouched = currentTouched;
    currentTouched = false;
    currentX = -1;
    currentY = -1;

    if (!available || spi == nullptr) return;
    if (config.irqPin >= 0 && gpio_get(static_cast<uint>(config.irqPin)) != 0) return;

    spi_set_baudrate(spi, config.spiFreq);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    int16_t data[6] = {};

    gpio_put(static_cast<uint>(config.csPin), 0);
    transferByte(COMMAND_Z1);
    const int16_t z1 = static_cast<int16_t>(transfer16(COMMAND_Z2) >> 3);
    const int16_t z2 = static_cast<int16_t>(transfer16(COMMAND_X) >> 3);
    const int32_t pressure = static_cast<int32_t>(z1) + ADC_MAX - z2;

    if (pressure >= config.minZ)
    {
        transfer16(COMMAND_X); // The first coordinate conversion is noisy.
        data[0] = static_cast<int16_t>(transfer16(COMMAND_Y) >> 3);
        data[1] = static_cast<int16_t>(transfer16(COMMAND_X) >> 3);
        data[2] = static_cast<int16_t>(transfer16(COMMAND_Y) >> 3);
        data[3] = static_cast<int16_t>(transfer16(COMMAND_X) >> 3);
    }

    data[4] = static_cast<int16_t>(transfer16(COMMAND_Y_POWER_DOWN) >> 3);
    data[5] = static_cast<int16_t>(transfer16(0) >> 3);
    gpio_put(static_cast<uint>(config.csPin), 1);

    if (pressure < config.minZ) return;

    const int16_t rawX = bestTwoAverage(data[0], data[2], data[4]);
    const int16_t rawY = bestTwoAverage(data[1], data[3], data[5]);
    const int16_t nativeX = mapClamped(rawX, config.minX, config.maxX, 0, static_cast<int16_t>(config.nativeWidth - 1));
    const int16_t nativeY = mapClamped(rawY, config.minY, config.maxY, 0, static_cast<int16_t>(config.nativeHeight - 1));

    switch (config.rotate & 3u)
    {
    case 0:
        currentX = nativeX;
        currentY = nativeY;
        break;
    case 1:
        currentX = static_cast<int16_t>(config.nativeHeight - 1) - nativeY;
        currentY = nativeX;
        break;
    case 2:
        currentX = static_cast<int16_t>(config.nativeWidth - 1) - nativeX;
        currentY = static_cast<int16_t>(config.nativeHeight - 1) - nativeY;
        break;
    default:
        currentX = nativeY;
        currentY = static_cast<int16_t>(config.nativeWidth - 1) - nativeX;
        break;
    }

    currentTouched = true;
}