#ifndef WAVEGUIDE_SDR_SAMPLETHREAD_H
#define WAVEGUIDE_SDR_SAMPLETHREAD_H

#include <thread>
#include <string>
#include <chrono>
#include <cstdint>

#include "SpectrumSamples.h"

class Config;

namespace sdr {

    class SampleThread {
    public:
        SampleThread(Config* config, uint8_t device_id, uint64_t start_freq_hz, uint64_t end_freq_hz, SpectrumSamples* samples_);
        ~SampleThread();

        void operator()();
        bool start();
        bool stop();

    private:
        std::thread* thread_;
        Config* config_;
        SpectrumSamples* samples_;

        std::string device_type_;
        uint8_t device_id_;

        uint64_t start_freq_hz_;
        uint64_t end_freq_hz_;
        uint64_t sample_rate_hz_;
        uint64_t sweep_count_;                      // how many total sweeps from start_freq_hz_ to end_freq_hz_ have been done?

        uint32_t dwell_time_us_;                    // how long to dwell on each tuned center freq (split into n FFT iterations)
        std::chrono::high_resolution_clock::time_point last_retuned_at_;

        bool stop_;
    };

}

#endif  // WAVEGUIDE_SDR_SAMPLETHREAD_H
