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
    .clkPin   = 14,
    .dataPin  = 15,  // mosi
    .dcPin    = 3,
    .csPin    = 1,
    .resetPin = 2,
    .backlightPin = -1,

    // ===== Display =====
    .lcdRotate = 1 ,  // 0: Normal  3: Inverted   

    // ===== Buffer =====
    .maxBufferWidth  = PLAMIO::Graphics::ILI9341_SCREEN_BUF_W_MAX_RP2040,
    .maxBufferHeight = PLAMIO::Graphics::ILI9341_SCREEN_BUF_H_MAX_RP2040
};
#elif PLAMIO_DISPLAY_SSD1306
/// SSD1306 OLED
constexpr PLAMIO::GraphicsSSD1306::Config GRAPHICS_CONFIG {
    // ===== I2C =====
    .i2cPort = 0,      // 0 or 1
    .i2cAddr = 0x3C,

    // ===== OLED Pins =====
    .sdaPin = 0,
    .sclPin = 1,
    .resetPin = -1,

    // ===== Display =====
    .oledRotate = 2  // 0: Normal  2: Inverted
};
#endif


//== Input ==================================================
/// Button-GPIO Mapping
constexpr PLAMIO::InputBase::ButtonMapping BUTTON_MAPPING {
    .UP        = 9,
    .DOWN      = 10,
    .LEFT      = 11,
    .RIGHT     = 12,
    .A         = 13,
    .B         = -1,
    .L         = -1,
    .R         = -1,
    .START     = -1,
    .SELECT    = -1,
    .VOL_UP    = -1,
    .VOL_DOWN  = -1,
    .HOME      = 27,
    .MUTE      = -1
};

#if PLAMIO_INPUT_SNES
/// SNES controller
constexpr PLAMIO::InputSnes::Config INPUT_CONFIG {
    // ===== GPIO Pins =====
    .gpioCLK = -1,
    .gpioLAT = -1,
    .gpioData = -1,

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


//== Audio ====================================================
#if PLAMIO_AUDIO_PWM
/// PWM
constexpr PLAMIO::AudioPWM::Config AUDIO_CONFIG {
    // ===== GPIO Pins =====
    .pwmPin = 29 
};
#elif PLAMIO_AUDIO_I2S
/// I2S
constexpr PLAMIO::AudioI2S::Config AUDIO_CONFIG {
    // ===== I2S Pins =====
    .bclkPin = -1,
    // The LRCK pin must be assigned to the BCLK + 1 pin.
    .dataPin = -1 
};
#endif


//== Storage ==================================================
#if PLAMIO_STORAGE_SD
// SD
constexpr PLAMIO::StorageSD::Config STORAGE_CONFIG {
    // ===== SPI =====
    .spiHost = 0,

    // ===== SD Card Pins =====
    .misoPin = 4,
    .sckPin  = 6,  // SCK or CLK
    .mosiPin = 7,
    .csPin   = 5,

    // ===== Speed =====
    .baudRate = 12 * 1000 * 1000
};
#endif


//== Battery =================================================
constexpr PLAMIO::PicoPlatform::BatteryConfig BATTERY_CONFIG {
    // ===== ADC =====
    .adcPin = -1,
    .adcChannel = 2,

    // ===== Voltage Thresholds =====
    .externalPowerThresholdVoltage = 2.0f,
    .warningVoltage = 3.6f,
    .criticalVoltage = 3.5f,
    .cutoffVoltage = 3.4f
};
