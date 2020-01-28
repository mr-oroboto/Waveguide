#ifndef WAVEGUIDE_SDR_SPECTRUMSAMPLER_H
#define WAVEGUIDE_SDR_SPECTRUMSAMPLER_H

#include <vector>
#include <thread>
#include <cstdint>

#include "SampleThread.h"
#include "SpectrumSamples.h"

namespace sdr {

    class SpectrumSampler {
    public:
        SpectrumSampler(uint8_t device_count, uint64_t capture_device_sample_rate_hz);
        ~SpectrumSampler();

        bool start(uint64_t start_freq_hz, uint64_t end_freq_hz);
        void stop();

        SpectrumSamples* getSamples();

    private:
        uint8_t device_count_;             // number of devices to split the total bandwidth over
        uint64_t capture_device_sample_rate_hz_;

        std::vector<SampleThread*> sample_threads_;
        SpectrumSamples* samples_;
    };

}

#endif //WAVEGUIDE_SDR_SPECTRUMSAMPLER_H
