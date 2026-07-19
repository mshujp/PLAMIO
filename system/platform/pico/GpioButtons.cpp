#include "GpioButtons.h"
#include "pico/stdlib.h"

using namespace PLAMIO;

void GpioButtons::initPin(int16_t pin)
{
    if (pin < 0)
    {
        return;
    }

    const uint gpio = static_cast<uint>(pin);
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

bool GpioButtons::isPressed(int16_t pin)
{
    if (pin < 0)
    {
        return false;
    }

    return !gpio_get(static_cast<uint>(pin));
}

void GpioButtons::init(const InputBase::ButtonMapping& mapping)
{
    initPin(mapping.UP);
    initPin(mapping.DOWN);
    initPin(mapping.LEFT);
    initPin(mapping.RIGHT);
    initPin(mapping.A);
    initPin(mapping.B);
    initPin(mapping.X);
    initPin(mapping.Y);
    initPin(mapping.L);
    initPin(mapping.R);
    initPin(mapping.START);
    initPin(mapping.SELECT);
    initPin(mapping.VOL_UP);
    initPin(mapping.VOL_DOWN);
    initPin(mapping.HOME);
    initPin(mapping.MUTE);
}

uint32_t GpioButtons::read(const InputBase::ButtonMapping& mapping)
{
    uint32_t buttons = 0;

    if (isPressed(mapping.UP))       buttons |= static_cast<uint32_t>(Input::Button::UP);
    if (isPressed(mapping.DOWN))     buttons |= static_cast<uint32_t>(Input::Button::DOWN);
    if (isPressed(mapping.LEFT))     buttons |= static_cast<uint32_t>(Input::Button::LEFT);
    if (isPressed(mapping.RIGHT))    buttons |= static_cast<uint32_t>(Input::Button::RIGHT);
    if (isPressed(mapping.A))        buttons |= static_cast<uint32_t>(Input::Button::A);
    if (isPressed(mapping.B))        buttons |= static_cast<uint32_t>(Input::Button::B);
    if (isPressed(mapping.X))        buttons |= static_cast<uint32_t>(Input::Button::X);
    if (isPressed(mapping.Y))        buttons |= static_cast<uint32_t>(Input::Button::Y);
    if (isPressed(mapping.L))        buttons |= static_cast<uint32_t>(Input::Button::L);
    if (isPressed(mapping.R))        buttons |= static_cast<uint32_t>(Input::Button::R);
    if (isPressed(mapping.START))    buttons |= static_cast<uint32_t>(Input::Button::START);
    if (isPressed(mapping.SELECT))   buttons |= static_cast<uint32_t>(Input::Button::SELECT);
    if (isPressed(mapping.VOL_UP))   buttons |= static_cast<uint32_t>(Input::Button::VOL_UP);
    if (isPressed(mapping.VOL_DOWN)) buttons |= static_cast<uint32_t>(Input::Button::VOL_DOWN);
    if (isPressed(mapping.HOME))     buttons |= static_cast<uint32_t>(Input::Button::HOME);
    if (isPressed(mapping.MUTE))     buttons |= static_cast<uint32_t>(Input::Button::MUTE);

    return buttons;
}
