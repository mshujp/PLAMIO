#pragma once

#include "AudioBase.h"

namespace PLAMIO {

class AudioStub : public AudioBase {
private:
    static constexpr uint16_t SAMPLE_RATE = 22050;

    uint32_t sampleRate() const override;
    bool toneSamples(int startFrequency, int endFrequency, uint32_t totalSamples, uint32_t& writtenSamples, float startVolumeScale, float endVolumeScale) override;

public:
    const char* getName() const override { return "NONE"; }
    uint8_t getVolumeSteps() const override { return 0; }
    bool begin() override;
    void end() override;
};

} // namespace PLAMIO
