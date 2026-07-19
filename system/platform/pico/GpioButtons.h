#pragma once

#include "InputBase.h"
#include <stdint.h>

namespace PLAMIO {

// Shared helper that interprets ButtonMapping values as GPIO numbers.
// Buttons are active-low and use the MCU's internal pull-up resistors.
class GpioButtons
{
public:
    static void init(const InputBase::ButtonMapping& mapping);
    static uint32_t read(const InputBase::ButtonMapping& mapping);

private:
    static void initPin(int16_t pin);
    static bool isPressed(int16_t pin);
};

} // namespace PLAMIO
