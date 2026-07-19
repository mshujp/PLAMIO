#pragma once

#include "AudioBase.h"
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"

namespace PLAMIO {

class AudioI2S : public AudioBase
{
    static constexpr uint16_t SAMPLE_RATE = 22050;
    static constexpr uint16_t SAMPLES_PER_BUFFER = 256;
    static constexpr uint8_t BUFFER_COUNT = 3;
    static constexpr uint8_t DMA_CHANNEL = 11;

    const audio_format_t audioFormat = {
        .sample_freq = SAMPLE_RATE,
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .channel_count = 2
    };
    audio_buffer_format_t producerFormat = {
        .format = &audioFormat,
        .sample_stride = 4
    };
    audio_i2s_config_t config;
    audio_buffer_pool_t* producerPool = nullptr;
    float phase = 0.0f;
    bool started = false;

    uint32_t sampleRate() const override;
    bool toneSamples(int startFrequency, int endFrequency, uint32_t totalSamples, uint32_t& writtenSamples, float startVolumeScale, float endVolumeScale) override;

public:
    struct Config {
        int8_t bclkPin = -1;
        int8_t dataPin = -1;
    };


    explicit AudioI2S(const Config& config);
    const char* getName() const override { return "I2S"; }
    uint8_t getVolumeSteps() const override { return 4; }
    bool begin() override;
    void end() override;
};

} // namespace PLAMIO
