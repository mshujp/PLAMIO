#pragma once

#include "BusParallelPIO.h"
#include "GraphicsILI9341.h"
#include "LGFXContext.h"

namespace PRUZEA {

class LGFXContextParallel final : public LGFXContext
{
private:
    BusParallelPIO bus;

public:
    explicit LGFXContextParallel(const GraphicsILI9341::GraphicsILI9341ParallelConfig& config) : bus(config)
    {
        configurePanel(bus, config.csPin, config.resetPin, false);
        finishConfiguration();
    }

    void enableTouch(const InputTouchConfig& config) override
    {
        configureTouch(config, false);
    }
};

} // namespace PRUZEA
