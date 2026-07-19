#include "InputPS2.h"
#include "GpioButtons.h"
#include "Platform.h"
#include "pico/stdlib.h"

using namespace PLAMIO;

namespace
{
constexpr uint32_t buttonMask(Input::Button button)
{
    return static_cast<uint32_t>(button);
}

constexpr uint8_t TRANSFER_HALF_PERIOD_USEC = 4;
constexpr uint8_t ATTENTION_SETUP_USEC = 10;

enum PS2Button : uint16_t
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

InputPS2::InputPS2(const Config& config)
    : clockPin(config.clockPin),
      commandPin(config.commandPin),
      attentionPin(config.attentionPin),
      dataPin(config.dataPin),
      buttonMapping(config.buttonMapping)
{
}

bool InputPS2::begin()
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

    uint32_t buttons = 0;
    available = pollController(buttons);
    return available;
}

void InputPS2::end()
{
    available = false;
    if (attentionPin >= 0) gpio_put(static_cast<uint>(attentionPin), 1);
    if (clockPin >= 0) gpio_put(static_cast<uint>(clockPin), 1);
    if (commandPin >= 0) gpio_put(static_cast<uint>(commandPin), 1);
    reset();
}

uint8_t InputPS2::transferByte(uint8_t command)
{
    uint8_t response = 0;
    for (uint8_t bit = 0; bit < 8; ++bit)
    {
        gpio_put(static_cast<uint>(commandPin), (command >> bit) & 1u);
        gpio_put(static_cast<uint>(clockPin), 0);
        Platform::sleepUsec(TRANSFER_HALF_PERIOD_USEC);

        if (gpio_get(static_cast<uint>(dataPin)))
        {
            response |= static_cast<uint8_t>(1u << bit);
        }

        gpio_put(static_cast<uint>(clockPin), 1);
        Platform::sleepUsec(TRANSFER_HALF_PERIOD_USEC);
    }
    gpio_put(static_cast<uint>(commandPin), 1);
    return response;
}

bool InputPS2::pollController(uint32_t& buttons)
{
    static constexpr uint8_t COMMAND[] = {0x01, 0x42, 0x00, 0x00, 0x00};
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
        buttons = 0;
        return false;
    }

    const uint16_t pressed = static_cast<uint16_t>(~(
        static_cast<uint16_t>(response[3]) |
        static_cast<uint16_t>(static_cast<uint16_t>(response[4]) << 8)));

    buttons = 0;
    if (pressed & PS2Button::UP)       buttons |= buttonMask(Button::UP);
    if (pressed & PS2Button::DOWN)     buttons |= buttonMask(Button::DOWN);
    if (pressed & PS2Button::LEFT)     buttons |= buttonMask(Button::LEFT);
    if (pressed & PS2Button::RIGHT)    buttons |= buttonMask(Button::RIGHT);
    if (pressed & PS2Button::CROSS)    buttons |= buttonMask(Button::A);
    if (pressed & PS2Button::CIRCLE)   buttons |= buttonMask(Button::B);
    if (pressed & PS2Button::SQUARE)   buttons |= buttonMask(Button::X);
    if (pressed & PS2Button::TRIANGLE) buttons |= buttonMask(Button::Y);
    if (pressed & PS2Button::L1)       buttons |= buttonMask(Button::L);
    if (pressed & PS2Button::R1)       buttons |= buttonMask(Button::R);
    if (pressed & PS2Button::START)    buttons |= buttonMask(Button::START);
    if (pressed & PS2Button::SELECT)   buttons |= buttonMask(Button::SELECT);

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

    buttons |= GpioButtons::read(buttonMapping);
    return true;
}

uint32_t InputPS2::readButtons()
{
    uint32_t buttons = 0;
    pollController(buttons);
    return buttons;
}
