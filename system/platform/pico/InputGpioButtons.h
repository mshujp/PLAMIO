#pragma once

#include "InputBase.h"

namespace PLAMIO {

// Input implementation for buttons individually connected to GPIO pins.
class InputGpioButtons : public InputBase
{
private:
    ButtonMapping buttonMapping;

    uint32_t readButtons() override;

public:
    explicit InputGpioButtons(const ButtonMapping& buttonMapping);

    const char* getName() const override { return "GPIO BUTTONS"; }
    bool begin() override;
    void end() override;
};

} // namespace PLAMIO
