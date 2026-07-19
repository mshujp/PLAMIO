#include "InputGpioButtons.h"
#include "GpioButtons.h"

using namespace PLAMIO;

InputGpioButtons::InputGpioButtons(const ButtonMapping& _buttonMapping)
    : buttonMapping(_buttonMapping)
{
}

bool InputGpioButtons::begin()
{
    GpioButtons::init(buttonMapping);
    reset();
    available = true;
    return true;
}

void InputGpioButtons::end()
{
    available = false;
    reset();
}

uint32_t InputGpioButtons::readButtons()
{
    return GpioButtons::read(buttonMapping);
}
