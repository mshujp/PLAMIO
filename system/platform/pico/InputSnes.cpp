#include "InputSnes.h"
#include "GpioButtons.h"
#include "Platform.h"
#include "pico/stdlib.h"

using namespace PLAMIO;

namespace
{
constexpr Input::Button SNES_BUTTON_MAP[] = {
    Input::Button::B,
    Input::Button::Y,
    Input::Button::SELECT,
    Input::Button::START,
    Input::Button::UP,
    Input::Button::DOWN,
    Input::Button::LEFT,
    Input::Button::RIGHT,
    Input::Button::A,
    Input::Button::X,
    Input::Button::L,
    Input::Button::R
};

constexpr size_t SNES_BUTTON_COUNT = sizeof(SNES_BUTTON_MAP) / sizeof(SNES_BUTTON_MAP[0]);
}

InputSnes::InputSnes(const Config& config)
    : gpioCLK(config.gpioCLK),
      gpioLAT(config.gpioLAT),
      gpioDATA(config.gpioData),
      buttonMapping(config.buttonMapping)
{
}

bool InputSnes::begin()
{
    gpio_init(static_cast<uint>(gpioCLK));
    gpio_set_dir(static_cast<uint>(gpioCLK), GPIO_OUT);

    gpio_init(static_cast<uint>(gpioLAT));
    gpio_set_dir(static_cast<uint>(gpioLAT), GPIO_OUT);

    gpio_init(static_cast<uint>(gpioDATA));
    gpio_set_dir(static_cast<uint>(gpioDATA), GPIO_IN);
    gpio_pull_up(static_cast<uint>(gpioDATA));

    GpioButtons::init(buttonMapping);

    reset();
    available = true;
    return true;
}

void InputSnes::end()
{
    available = false;
    reset();
}

uint32_t InputSnes::readButtons()
{
    uint32_t buttons = 0;

    gpio_put(static_cast<uint>(gpioCLK), 0);

    gpio_put(static_cast<uint>(gpioLAT), 1);
    Platform::sleepUsec(12);

    gpio_put(static_cast<uint>(gpioLAT), 0);
    Platform::sleepUsec(6);

    for (size_t i = 0; i < 16; ++i)
    {
        if (i < SNES_BUTTON_COUNT && !gpio_get(static_cast<uint>(gpioDATA)))
        {
            buttons |= static_cast<uint32_t>(SNES_BUTTON_MAP[i]);
        }

        gpio_put(static_cast<uint>(gpioCLK), 1);
        Platform::sleepUsec(6);

        gpio_put(static_cast<uint>(gpioCLK), 0);
        Platform::sleepUsec(6);
    }

    if (buttonMapping.VOL_DOWN < 0)
    {
        // L acts as VOL_DOWN.
        if (buttons & static_cast<uint32_t>(Button::L))
        {
            buttons |= static_cast<uint32_t>(Button::VOL_DOWN);
            buttons &= ~static_cast<uint32_t>(Button::L);
        }
    }
    if (buttonMapping.VOL_UP < 0)
    {
        // R acts as VOL_UP.
        if (buttons & static_cast<uint32_t>(Button::R))
        {
            buttons |= static_cast<uint32_t>(Button::VOL_UP);
            buttons &= ~static_cast<uint32_t>(Button::R);
        }
    }

    if (buttonMapping.HOME < 0)
    {
        // SELECT + START acts as HOME.
        if ((buttons & static_cast<uint32_t>(Button::SELECT)) &&
            (buttons & static_cast<uint32_t>(Button::START)))
        {
            buttons &= ~static_cast<uint32_t>(Button::SELECT);
            buttons &= ~static_cast<uint32_t>(Button::START);
            buttons |= static_cast<uint32_t>(Button::HOME);
        }
    }

    // Auxiliary GPIO buttons are already mapped to their final logical roles.
    buttons |= GpioButtons::read(buttonMapping);

    return buttons;
}
