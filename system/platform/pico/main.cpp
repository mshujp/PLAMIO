#include "System.h"
#include "PLAMIOConfig.h"
#include "PicoBatteryConfig.h"

#if PLAMIO_INPUT_GPIO_BUTTONS
#include "InputGpioButtons.h"
#elif PLAMIO_INPUT_SNES
#include "InputSnes.h"
#elif PLAMIO_INPUT_PS2
#include "InputPS2.h"
#endif

#if PLAMIO_AUDIO_I2S
#include "AudioI2S.h"
#elif PLAMIO_AUDIO_PWM
#include "AudioPWM.h"
#elif PLAMIO_AUDIO_NONE
#include "AudioStub.h"
#endif

#if PLAMIO_DISPLAY_ILI9341
#include "GraphicsILI9341.h"
#include "SystemUI320x240.h"
#include "SystemUI128x64Mono.h"
#elif PLAMIO_DISPLAY_SSD1306
#include "GraphicsSSD1306.h"
#include "SystemUI128x64Mono.h"
#endif

#if PLAMIO_STORAGE_SD
#include "StorageSD.h"
#elif PLAMIO_STORAGE_NONE
#include "StorageStub.h"
#endif

#include PLAMIO_BOARD_CONFIG_HEADER

#include <hardware/adc.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>

using namespace PLAMIO;

namespace
{

#if PLAMIO_DISPLAY_ILI9341
GraphicsILI9341 graphicsImpl(GRAPHICS_CONFIG);
SystemUI320x240 systemUIImpl;
//SystemUI128x64Mono systemUIImpl(Graphics::Color::SSD1306_ON, Graphics::Color::SSD1306_OFF);
#elif PLAMIO_DISPLAY_SSD1306
GraphicsSSD1306 graphicsImpl(GRAPHICS_CONFIG);
SystemUI128x64Mono systemUIImpl(Graphics::Color::SSD1306_ON, Graphics::Color::SSD1306_OFF);
#endif

#if PLAMIO_INPUT_GPIO_BUTTONS
InputGpioButtons inputImpl(BUTTON_MAPPING);
#elif PLAMIO_INPUT_SNES
InputSnes inputImpl(INPUT_CONFIG);
#elif PLAMIO_INPUT_PS2
InputPS2 inputImpl(INPUT_CONFIG);
#endif

#if PLAMIO_STORAGE_SD
StorageSD storageImpl(STORAGE_CONFIG);
#elif PLAMIO_STORAGE_NONE
StorageStub storageImpl;
#endif

#if PLAMIO_AUDIO_I2S
AudioI2S audioImpl(AUDIO_CONFIG);
#elif PLAMIO_AUDIO_PWM
AudioPWM audioImpl(AUDIO_CONFIG);
#elif PLAMIO_AUDIO_NONE
AudioStub audioImpl;
#endif

System* activeSystem = nullptr;

void initPlatformHardware()
{
    if (BATTERY_CONFIG.adcPin >= 0)
    {
        const uint batteryPin = static_cast<uint>(BATTERY_CONFIG.adcPin);
        gpio_disable_pulls(batteryPin);
        adc_init();
        adc_gpio_init(batteryPin);
        gpio_set_dir(batteryPin, GPIO_IN);
    }

#if PLAMIO_DISPLAY_ILI9341
    if (GRAPHICS_CONFIG.backlightPin >= 0)
    {
        gpio_init(static_cast<uint>(GRAPHICS_CONFIG.backlightPin));
        gpio_set_dir(static_cast<uint>(GRAPHICS_CONFIG.backlightPin), GPIO_OUT);
        gpio_put(static_cast<uint>(GRAPHICS_CONFIG.backlightPin), 1);
    }
#endif
}

bool readBatteryVoltage(void*, float& voltage)
{
    if (BATTERY_CONFIG.adcPin < 0)
    {
        return false;
    }

    adc_select_input(BATTERY_CONFIG.adcChannel);
    const uint16_t raw = adc_read();
    voltage = (raw * 3.3f / 4095.0f) * 2.0f;
    return true;
}

void __time_critical_func(audioCoreEntry)()
{
    if (activeSystem != nullptr)
    {
        activeSystem->runAudioWorker();
    }
}

bool launchAudioWorker(void*, System& system)
{
    activeSystem = &system;
    multicore_launch_core1(audioCoreEntry);
    return true;
}

void finishAudioWorker(void*, bool)
{
    multicore_reset_core1();
    activeSystem = nullptr;
}

void enterPowerOffState(void*, System::ShutdownReason)
{
    __asm volatile("cpsid i");
    while (true)
    {
        __wfi();
    }
}

} // namespace

int main()
{
    stdio_init_all();
    initPlatformHardware();

    const System::Callbacks callbacks{
        nullptr,
        readBatteryVoltage,
        launchAudioWorker,
        finishAudioWorker,
        enterPowerOffState
    };

    const System::BatteryConfig batteryConfig{
        .externalPowerThreshold = BATTERY_CONFIG.externalPowerThresholdVoltage,
        .warningThreshold = BATTERY_CONFIG.warningVoltage,
        .criticalThreshold = BATTERY_CONFIG.criticalVoltage,
        .cutoffThreshold = BATTERY_CONFIG.cutoffVoltage,
        .sampleIntervalFrames = 300,
        .initialSampleCount = 10,
        .initialSampleDelayMsec = 50
    };

    System system(
        graphicsImpl,
        inputImpl,
        storageImpl,
        audioImpl,
        systemUIImpl,
        callbacks,
        batteryConfig);

    return system.start() ? 0 : 1;
}
