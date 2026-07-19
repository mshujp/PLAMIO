#pragma once

#include "InputBase.h"

//== Graphics ================================================
#if PLAMIO_DISPLAY_ILI9341
/// ILI9341 SPI LCD
constexpr PLAMIO::GraphicsILI9341::Config GRAPHICS_CONFIG {
    // ===== SPI =====
    .spiHost = 1,
    .spiWriteFreq = 62500000,

    // ===== LCD Pins =====
    .clkPin = 10,
    .dataPin = 11,  // mosi
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
    // ===== I2C =====
    .i2cPort = 0,
    .i2cAddr = 0x3C,

    // ===== OLED Pins =====
    .sdaPin = -1,
    .sclPin = -1,
    .resetPin = -1,

    // ===== Display =====
    .oledRotate = 0
};
#endif


//== Input ===================================================
/// Button-GPIO Mapping
constexpr PLAMIO::InputBase::ButtonMapping BUTTON_MAPPING {
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
/// PWM
constexpr PLAMIO::AudioPWM::Config AUDIO_CONFIG {
    // ===== GPIO Pins =====
    .pwmPin = -1
};
#elif PLAMIO_AUDIO_I2S
/// I2S
constexpr PLAMIO::AudioI2S::Config AUDIO_CONFIG {
    // ===== I2S Pins =====
    .bclkPin = 4,
    // The LRCK pin must be assigned to the BCLK + 1 pin.
    .dataPin = 2
};
#endif


//== Storage =================================================
#if PLAMIO_STORAGE_SD
// SD
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
constexpr PLAMIO::PicoPlatform::BatteryConfig BATTERY_CONFIG {
    // ===== ADC =====
    .adcPin = 42,
    .adcChannel = 2,

    // ===== Voltage Thresholds =====
    .externalPowerThresholdVoltage = 2.0f,
    .warningVoltage = 3.6f,
    .criticalVoltage = 3.5f,
    .cutoffVoltage = 3.4f
};
