#ifndef WAVEGUIDE_SDR_SAMPLETHREAD_H
#define WAVEGUIDE_SDR_SAMPLETHREAD_H

#include <thread>
#include <string>
#include <cstdint>

#include "SpectrumSamples.h"

namespace sdr {

    class SampleThread {
    public:
        SampleThread(const std::string& device_type, uint8_t device_id, uint64_t start_freq_hz, uint64_t end_freq_hz,
                     uint64_t sample_rate_hz, SpectrumSamples* samples_);
        ~SampleThread();

        void operator()();
        bool start();
        bool stop();

    private:
        std::thread* thread_;
        SpectrumSamples* samples_;

        std::string device_type_;
        uint8_t device_id_;

        uint64_t start_freq_hz_;
        uint64_t end_freq_hz_;
        uint64_t sample_rate_hz_;
        uint32_t dwell_time_us_;                    // how long to dwell on each tuned center freq (split into n FFT iterations)
        uint16_t iterations_per_dwell_time_;        // how many FFT iterations to perform over the course of a dwell
        uint64_t sweep_count_;                      // how many total sweeps from start_freq_hz_ to end_freq_hz_ have been done?

        bool stop_;
    };

}

#endif  // WAVEGUIDE_SDR_SAMPLETHREAD_H
