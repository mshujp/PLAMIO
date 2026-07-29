#pragma once

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

} // namespace PRUZEA
