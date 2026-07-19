#pragma once

#include "InputBase.h"

namespace PLAMIO {

class InputSnes : public InputBase
{
private:
    int8_t gpioCLK;
    int8_t gpioLAT;
    int8_t gpioDATA;
    ButtonMapping buttonMapping;

    uint32_t readButtons() override;

public:
    struct Config {
        int8_t gpioCLK  = -1;
        int8_t gpioLAT  = -1;
        int8_t gpioData = -1;
        ButtonMapping buttonMapping{};
    };

    explicit InputSnes(const Config& config);

    const char* getName() const override { return "SNES PAD"; }
    bool begin() override;
    void end() override;
};

} // namespace PLAMIO
