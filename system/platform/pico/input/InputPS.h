#pragma once

#include "InputBase.h"

namespace PRUZEA {

// Experimental PlayStation controller input implementation.
// Compile-tested only; communication with real hardware has not been verified.
// ACK, vibration, and mode setup are intentionally not implemented.
class InputPS : public InputBase
{
private:
    int8_t clockPin;
    int8_t commandPin;
    int8_t attentionPin;
    int8_t dataPin;
    ButtonMapping buttonMapping;
    uint8_t analogDeadZone;
    uint8_t axisCenters[4];
    uint8_t axisValues[4]{};
    bool analogAvailable = false;

    uint8_t transferByte(uint8_t command);
    bool pollController(uint32_t& buttons);
    uint32_t readButtons() override;

public:
    struct Config
    {
        int8_t clockPin = -1;
        int8_t commandPin = -1;
        int8_t attentionPin = -1;
        int8_t dataPin = -1;
        ButtonMapping buttonMapping{};
        uint8_t analogDeadZone = 20;
        uint8_t leftXCenter = 128;
        uint8_t leftYCenter = 128;
        uint8_t rightXCenter = 128;
        uint8_t rightYCenter = 128;
    };

    explicit InputPS(const Config& config);

    const char* getName() const override { return "PS PAD (EXPERIMENTAL)"; }
    bool begin() override;
    void end() override;
    bool hasAnalogSticks() const override;
    int16_t axis(Axis axis) const override;
};

} // namespace PRUZEA
