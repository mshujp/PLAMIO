#pragma once

#include "InputBase.h"

// Raspberry Pi Pico 2 (RP2350) sample hardware profile.
// Change each GPIO number to match the actual wiring.
// Use -1 for hardware that is not connected.

//== Graphics ================================================
#if PLAMIO_DISPLAY_ILI9341
/// ILI9341 SPI LCD
constexpr PLAMIO::GraphicsILI9341::Config GRAPHICS_CONFIG {
    // ===== SPI1 =====
    .spiHost = 1,
    .spiWriteFreq = 62500000,

    // ===== LCD Pins =====
    .clkPin = 10,
    .dataPin = 11,  // MOSI
    .dcPin = 14,
    .csPin = 9,
    .resetPin = 15,
    .backlightPin = 22,

    // ===== Display =====
    .lcdRotate = 3,  // 0: Normal  3: Inverted

    // ===== Buffer =====
    .maxBufferWidth = PLAMIO::Graphics::ILI9341_SCREEN_BUF_W_MAX_RP2350,
    .maxBufferHeight = PLAMIO::Graphics::ILI9341_SCREEN_BUF_H_MAX_RP2350
};
#elif PLAMIO_DISPLAY_SSD1306
/// SSD1306 OLED
constexpr PLAMIO::GraphicsSSD1306::Config GRAPHICS_CONFIG {
    // ===== I2C0 =====
    .i2cPort = 0,
    .i2cAddr = 0x3C,

    // ===== OLED Pins =====
    .sdaPin = 0,
    .sclPin = 1,
    .resetPin = -1,

    // ===== Display =====
    .oledRotate = 0
};
#endif


//== Input ===================================================
/// Direct GPIO buttons and system-button mapping.
constexpr PLAMIO::InputBase::ButtonMapping BUTTON_MAPPING {
    .UP = -1,
    .DOWN = -1,
    .LEFT = -1,
    .RIGHT = -1,
    .A = -1,
    .B = -1,
    .L = -1,
    .R = -1,
    .START = -1,
    .SELECT = -1,
    .VOL_UP = -1,
    .VOL_DOWN = -1,
    .HOME = -1,
    .MUTE = -1
};

#if PLAMIO_INPUT_SNES
/// SNES controller
constexpr PLAMIO::InputSnes::Config INPUT_CONFIG {
    // ===== GPIO Pins =====
    .gpioCLK = 27,
    .gpioLAT = 26,
    .gpioData = 28,

    // ===== Extra Buttons =====
    .buttonMapping = BUTTON_MAPPING
};
#elif PLAMIO_INPUT_PS2
/// PlayStation 2 controller (experimental; hardware-unverified)
constexpr PLAMIO::InputPS2::Config INPUT_CONFIG {
    // ===== GPIO Pins =====
    .clockPin = -1,
    .commandPin = -1,
    .attentionPin = -1,
    .dataPin = -1,

    // ===== Extra Buttons =====
    .buttonMapping = BUTTON_MAPPING
};
#endif


//== Audio ===================================================
#if PLAMIO_AUDIO_PWM
/// PWM speaker
constexpr PLAMIO::AudioPWM::Config AUDIO_CONFIG {
    // ===== GPIO Pins =====
    .pwmPin = 3
};
#elif PLAMIO_AUDIO_I2S
/// I2S amplifier
constexpr PLAMIO::AudioI2S::Config AUDIO_CONFIG {
    // ===== I2S Pins =====
    .bclkPin = 4,
    // The LRCK pin must be assigned to the BCLK + 1 pin (GPIO 5).
    .dataPin = 2
};
#endif


//== Storage =================================================
#if PLAMIO_STORAGE_SD
/// SD card over SPI0
constexpr PLAMIO::StorageSD::Config STORAGE_CONFIG {
    // ===== SPI =====
    .spiHost = 0,

    // ===== SD Card Pins =====
    .misoPin = 16,
    .sckPin = 18,
    .mosiPin = 19,
    .csPin = 13,

    // ===== Speed =====
    .baudRate = 12 * 1000 * 1000
};
#endif


//== Battery =================================================
/// Disabled in this sample. Configure an ADC pin and thresholds when needed.
constexpr PLAMIO::PicoPlatform::BatteryConfig BATTERY_CONFIG {
    // ===== ADC =====
    .adcPin = -1,
    .adcChannel = 0,

    // ===== Voltage Thresholds =====
    .externalPowerThresholdVoltage = 2.0f,
    .warningVoltage = 3.6f,
    .criticalVoltage = 3.5f,
    .cutoffVoltage = 3.4f
};
