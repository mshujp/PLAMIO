#include "AudioI2S.h"
#include <algorithm>

using namespace PLAMIO;

AudioI2S::AudioI2S(const Config& audioConfig)
{
    config = {
        .data_pin = static_cast<uint8_t>(audioConfig.dataPin),
        .clock_pin_base = static_cast<uint8_t>(audioConfig.bclkPin),
        .dma_channel = DMA_CHANNEL,
        .pio_sm = 0
    };
}

bool AudioI2S::begin()
{
    if (started) return true;

    const audio_format_t* outputFormat = audio_i2s_setup(&audioFormat, &config);
    if (outputFormat == nullptr) return false;

    producerPool = audio_new_producer_pool(&producerFormat, BUFFER_COUNT, SAMPLES_PER_BUFFER);
    if (producerPool == nullptr) return false;

    if (!audio_i2s_connect(producerPool)) return false;

    phase = 0.0f;
    audio_i2s_set_enabled(true);
    started = true;
    return true;
}

void AudioI2S::end()
{
    if (!started) return;

    audio_i2s_set_enabled(false);
    phase = 0.0f;
    started = false;
}

uint32_t AudioI2S::sampleRate() const
{
    return SAMPLE_RATE;
}

bool AudioI2S::toneSamples(
    int startFrequency,
    int endFrequency,
    uint32_t totalSamples,
    uint32_t& writtenSamples,
    float startVolumeScale,
    float endVolumeScale)
{
    if (!started || producerPool == nullptr || totalSamples == 0) return true;

    int baseAmplitude = 0;
    switch (getVolumeLevel())
    {
    case 1: baseAmplitude = 600; break;
    case 2: baseAmplitude = 1500; break;
    case 3: baseAmplitude = 3000; break;
    default: baseAmplitude = 0; break;
    }

    const float clampedStartVolume = std::clamp(startVolumeScale, 0.0f, 1.0f);
    const float clampedEndVolume = std::clamp(endVolumeScale, 0.0f, 1.0f);

    while (writtenSamples < totalSamples)
    {
        audio_buffer_t* buffer = take_audio_buffer(producerPool, true);
        if (buffer == nullptr) return true;

        int16_t* samples = reinterpret_cast<int16_t*>(buffer->buffer->bytes);
        const uint32_t samplesToWrite = std::min<uint32_t>(
            buffer->max_sample_count,
            totalSamples - writtenSamples
        );

        for (uint32_t i = 0; i < samplesToWrite; ++i)
        {
            const uint32_t sampleIndex = writtenSamples + i;
            const float progress = totalSamples <= 1
                ? 1.0f
                : static_cast<float>(sampleIndex) / static_cast<float>(totalSamples - 1);

            const float frequency = static_cast<float>(startFrequency)
                + static_cast<float>(endFrequency - startFrequency) * progress;
            const float volumeScale = clampedStartVolume
                + (clampedEndVolume - clampedStartVolume) * progress;
            const int amplitude = static_cast<int>(baseAmplitude * volumeScale);

            int16_t sample = 0;
            if (frequency > 0.0f && amplitude > 0)
            {
                sample = phase < 0.5f
                    ? static_cast<int16_t>(-amplitude)
                    : static_cast<int16_t>(amplitude);

                phase += frequency / static_cast<float>(SAMPLE_RATE);
                while (phase >= 1.0f) phase -= 1.0f;
            }

            samples[i * 2] = sample;
            samples[i * 2 + 1] = sample;
        }

        buffer->sample_count = samplesToWrite;
        give_audio_buffer(producerPool, buffer);
        writtenSamples += samplesToWrite;

        if (hasPendingSE()) return false;
    }

    return true;
}
