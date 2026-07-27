#include "InputPS.h"
#include "GpioButtons.h"
#include "Platform.h"
#include "pico/stdlib.h"
#include <cstdio>

using namespace PRUZEA;

namespace
{
constexpr uint32_t buttonMask(Input::Button button)
{
    return static_cast<uint32_t>(button);
}

constexpr uint8_t TRANSFER_HALF_PERIOD_USEC = 10;
constexpr uint8_t ATTENTION_SETUP_USEC = 10;
constexpr uint8_t BYTE_DELAY_USEC = 10;

enum PSButton : uint16_t
{
    SELECT   = 1u << 0,
    L3       = 1u << 1,
    R3       = 1u << 2,
    START    = 1u << 3,
    UP       = 1u << 4,
    RIGHT    = 1u << 5,
    DOWN     = 1u << 6,
    LEFT     = 1u << 7,
    L2       = 1u << 8,
    R2       = 1u << 9,
    L1       = 1u << 10,
    R1       = 1u << 11,
    TRIANGLE = 1u << 12,
    CIRCLE   = 1u << 13,
    CROSS    = 1u << 14,
    SQUARE   = 1u << 15
};
}

InputPS::InputPS(const Config& config)
    : clockPin(config.clockPin),
      commandPin(config.commandPin),
      attentionPin(config.attentionPin),
      dataPin(config.dataPin),
      buttonMapping(config.buttonMapping),
      analogDeadZone(config.analogDeadZone),
      axisCenters{config.leftXCenter, config.leftYCenter, config.rightXCenter, config.rightYCenter}
{
}

bool InputPS::begin()
{
    if (clockPin < 0 || commandPin < 0 || attentionPin < 0 || dataPin < 0)
    {
        available = false;
        reset();
        return false;
    }

    gpio_init(static_cast<uint>(clockPin));
    gpio_set_dir(static_cast<uint>(clockPin), GPIO_OUT);
    gpio_put(static_cast<uint>(clockPin), 1);

    gpio_init(static_cast<uint>(commandPin));
    gpio_set_dir(static_cast<uint>(commandPin), GPIO_OUT);
    gpio_put(static_cast<uint>(commandPin), 1);

    gpio_init(static_cast<uint>(attentionPin));
    gpio_set_dir(static_cast<uint>(attentionPin), GPIO_OUT);
    gpio_put(static_cast<uint>(attentionPin), 1);

    gpio_init(static_cast<uint>(dataPin));
    gpio_set_dir(static_cast<uint>(dataPin), GPIO_IN);
    gpio_pull_up(static_cast<uint>(dataPin));

    GpioButtons::init(buttonMapping);
    reset();

    available = true;
    return true;
}

void InputPS::end()
{
    available = false;
    analogAvailable = false;
    if (attentionPin >= 0) gpio_put(static_cast<uint>(attentionPin), 1);
    if (clockPin >= 0) gpio_put(static_cast<uint>(clockPin), 1);
    if (commandPin >= 0) gpio_put(static_cast<uint>(commandPin), 1);
    reset();
}

uint8_t InputPS::transferByte(uint8_t command)
{
    uint8_t response = 0;
    for (uint8_t bit = 0; bit < 8; ++bit)
    {
        gpio_put(static_cast<uint>(commandPin), (command >> bit) & 1u);
        gpio_put(static_cast<uint>(clockPin), 0);
        busy_wait_us_32(TRANSFER_HALF_PERIOD_USEC);

        if (gpio_get(static_cast<uint>(dataPin)))
        {
            response |= static_cast<uint8_t>(1u << bit);
        }

        gpio_put(static_cast<uint>(clockPin), 1);
        busy_wait_us_32(TRANSFER_HALF_PERIOD_USEC);
    }
    gpio_put(static_cast<uint>(commandPin), 1);
    busy_wait_us_32(BYTE_DELAY_USEC);
    return response;
}

bool InputPS::pollController(uint32_t& buttons)
{
    static constexpr uint8_t COMMAND[] = {0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t response[sizeof(COMMAND)]{};

    gpio_put(static_cast<uint>(attentionPin), 0);
    Platform::sleepUsec(ATTENTION_SETUP_USEC);

    for (size_t i = 0; i < sizeof(COMMAND); ++i)
    {
        response[i] = transferByte(COMMAND[i]);
    }

    gpio_put(static_cast<uint>(attentionPin), 1);
    Platform::sleepUsec(ATTENTION_SETUP_USEC);

    if (response[1] == 0x00 || response[1] == 0xFF || response[2] != 0x5A)
    {
        analogAvailable = false;
        buttons = 0;
        return false;
    }

    analogAvailable = (response[1] & 0xF0u) == 0x70u;
    if (analogAvailable)
    {
        axisValues[static_cast<uint8_t>(Axis::LEFT_X)] = response[7];
        axisValues[static_cast<uint8_t>(Axis::LEFT_Y)] = response[8];
        axisValues[static_cast<uint8_t>(Axis::RIGHT_X)] = response[5];
        axisValues[static_cast<uint8_t>(Axis::RIGHT_Y)] = response[6];
    }

    const uint16_t pressed = static_cast<uint16_t>(~(
        static_cast<uint16_t>(response[3]) |
        static_cast<uint16_t>(static_cast<uint16_t>(response[4]) << 8)));

    buttons = 0;
    if (pressed & PSButton::UP)       buttons |= buttonMask(Button::UP);
    if (pressed & PSButton::DOWN)     buttons |= buttonMask(Button::DOWN);
    if (pressed & PSButton::LEFT)     buttons |= buttonMask(Button::LEFT);
    if (pressed & PSButton::RIGHT)    buttons |= buttonMask(Button::RIGHT);
    if (pressed & PSButton::CROSS)    buttons |= buttonMask(Button::A);
    if (pressed & PSButton::CIRCLE)   buttons |= buttonMask(Button::B);
    if (pressed & PSButton::SQUARE)   buttons |= buttonMask(Button::X);
    if (pressed & PSButton::TRIANGLE) buttons |= buttonMask(Button::Y);
    if (pressed & PSButton::L1)       buttons |= buttonMask(Button::L);
    if (pressed & PSButton::R1)       buttons |= buttonMask(Button::R);
    if (pressed & PSButton::L2)       buttons |= buttonMask(Button::L2);
    if (pressed & PSButton::R2)       buttons |= buttonMask(Button::R2);
    if (pressed & PSButton::L3)       buttons |= buttonMask(Button::L3);
    if (pressed & PSButton::R3)       buttons |= buttonMask(Button::R3);
    if (pressed & PSButton::START)    buttons |= buttonMask(Button::START);
    if (pressed & PSButton::SELECT)   buttons |= buttonMask(Button::SELECT);

    if (buttonMapping.VOL_DOWN < 0 && (buttons & buttonMask(Button::L)))
    {
        buttons |= buttonMask(Button::VOL_DOWN);
        buttons &= ~buttonMask(Button::L);
    }
    if (buttonMapping.VOL_UP < 0 && (buttons & buttonMask(Button::R)))
    {
        buttons |= buttonMask(Button::VOL_UP);
        buttons &= ~buttonMask(Button::R);
    }
    if (buttonMapping.HOME < 0 &&
        (buttons & buttonMask(Button::SELECT)) &&
        (buttons & buttonMask(Button::START)))
    {
        buttons &= ~buttonMask(Button::SELECT);
        buttons &= ~buttonMask(Button::START);
        buttons |= buttonMask(Button::HOME);
    }

    return true;
}

uint32_t InputPS::readButtons()
{
    uint32_t buttons = GpioButtons::read(buttonMapping);

    uint32_t controllerButtons = 0;
    if (pollController(controllerButtons))
    {
        buttons |= controllerButtons;
    }

    return buttons;
}

bool InputPS::hasAnalogSticks() const
{
    return analogAvailable;
}

int16_t InputPS::axis(Axis axis) const
{
    if (!analogAvailable) return 0;

    const uint8_t index = static_cast<uint8_t>(axis);
    if (index >= 4) return 0;

    const int32_t value = axisValues[index];
    const int32_t center = axisCenters[index];
    const int32_t lowerEdge = center - analogDeadZone;
    const int32_t upperEdge = center + analogDeadZone;

    if (value < lowerEdge)
    {
        if (lowerEdge <= 0) return -1000;
        return static_cast<int16_t>((value - lowerEdge) * 1000 / lowerEdge);
    }
    if (value > upperEdge)
    {
        if (upperEdge >= 255) return 1000;
        return static_cast<int16_t>((value - upperEdge) * 1000 / (255 - upperEdge));
    }
    return 0;
}
