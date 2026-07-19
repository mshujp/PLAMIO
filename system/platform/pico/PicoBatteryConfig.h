#pragma once

#include <cstdint>

namespace PLAMIO::PicoPlatform {

struct BatteryConfig
{
    int8_t adcPin = -1;
    uint8_t adcChannel = 0;

    float externalPowerThresholdVoltage = 0.0f;
    float warningVoltage = 0.0f;
    float criticalVoltage = 0.0f;
    float cutoffVoltage = 0.0f;
};

} // namespace PLAMIO::PicoPlatform
