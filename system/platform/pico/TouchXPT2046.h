#pragma once

#include <stdint.h>

struct spi_inst;
typedef struct spi_inst spi_inst_t;

namespace PRUZEA {

struct InputTouchConfig
{
    uint8_t spiHost = 0; // SPI0 is recommended. Shared-bus operation is not guaranteed.
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
    int16_t minZ = 200;

    uint16_t nativeWidth = 240;
    uint16_t nativeHeight = 320;
    uint8_t rotate = 1;
};

class TouchXPT2046
{
private:
    InputTouchConfig config;
    spi_inst_t* spi = nullptr;
    bool available = false;
    bool currentTouched = false;
    bool previousTouched = false;
    int16_t currentX = -1;
    int16_t currentY = -1;

    uint8_t transferByte(uint8_t data);
    uint16_t transfer16(uint16_t data);
    static int16_t bestTwoAverage(int16_t a, int16_t b, int16_t c);
    static int16_t mapClamped(int32_t value, int32_t inMin, int32_t inMax, int16_t outMin, int16_t outMax);

public:
    explicit TouchXPT2046(const InputTouchConfig& config);

    bool begin();
    void end();
    void update();

    bool touched() const { return currentTouched; }
    bool justTouched() const { return currentTouched && !previousTouched; }
    bool justReleased() const { return !currentTouched && previousTouched; }
    int16_t x() const { return currentTouched ? currentX : -1; }
    int16_t y() const { return currentTouched ? currentY : -1; }
};

} // namespace PRUZEA