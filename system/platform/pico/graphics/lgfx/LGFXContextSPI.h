#pragma once

#include "graphics/GraphicsILI9341.h"
#include "LGFXContext.h"

namespace PRUZEA {

class LGFXContextSPI final : public LGFXContext
{
private:
    lgfx::Bus_SPI bus;
    uint8_t spiHost;

public:
    explicit LGFXContextSPI(const GraphicsILI9341::GraphicsILI9341SPIConfig& config)
        : spiHost(config.spiHost)
    {
        auto busConfig = bus.config();
        busConfig.spi_host = static_cast<decltype(busConfig.spi_host)>(config.spiHost);
        busConfig.spi_mode = 0;
        busConfig.freq_write = config.spiWriteFreq;
        busConfig.freq_read = 16000000;
        busConfig.pin_sclk = config.clkPin;
        busConfig.pin_mosi = config.dataPin;
        busConfig.pin_miso = -1;
        busConfig.pin_dc = config.dcPin;
        bus.config(busConfig);

        configurePanel(bus, config.csPin, config.resetPin, true);
        finishConfiguration();
    }

    void enableTouch(const InputTouchConfig& config) override
    {
        const bool busShared = config.spiHost == spiHost;
        if (busShared)
        {
            auto busConfig = bus.config();
            busConfig.pin_miso = config.misoPin;
            bus.config(busConfig);
        }
        configureTouch(config, busShared);
    }
};

} // namespace PRUZEA
