#ifndef WAVEGUIDE_SDR_SPECTRUMSAMPLER_H
#define WAVEGUIDE_SDR_SPECTRUMSAMPLER_H

#include <vector>
#include <thread>
#include <cstdint>

#include "SampleThread.h"
#include "SpectrumSamples.h"

class Config;

namespace sdr {

    class SpectrumSampler {
    public:
        SpectrumSampler(Config* config);
        ~SpectrumSampler();

        bool start(uint64_t start_freq_hz, uint64_t end_freq_hz);
        void stop();

        uint64_t getStartFrequency();
        uint64_t getEndFrequency();

        SpectrumSamples* getSamples();

    private:
        Config* config_;

        uint8_t device_count_;             // number of devices to split the total bandwidth over
        uint64_t capture_device_sample_rate_hz_;
        uint64_t start_freq_hz_;
        uint64_t end_freq_hz_;

        std::vector<SampleThread*> sample_threads_;
        SpectrumSamples* samples_;
    };

}

#endif //WAVEGUIDE_SDR_SPECTRUMSAMPLER_H
