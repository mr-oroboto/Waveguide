#ifndef WAVEGUIDE_SDR_FREQUENCYBIN_H
#define WAVEGUIDE_SDR_FREQUENCYBIN_H

#include <mutex>
#include <cstdint>

class SpectrumSamples;

namespace sdr {

    class FrequencyBin {
    public:
        FrequencyBin(uint64_t freq_hz, uint16_t history_size);
        ~FrequencyBin();

        uint64_t getFrequency();

        bool getHasBeenSet();
        float getLatestAmplitude(bool moving_average = true);
        float getMaximumAmplitude();

    private:
        friend class SpectrumSamples;

        void setLatestAmplitude(float amplitude, bool keep_maximum);

        std::mutex lock_;

        uint64_t freq_hz_;                  // frequency the sample represents
        float max_amplitude_;               // maximum amplitude seen for this frequency (ever)
        float moving_average_amplitude_;

        uint16_t history_size_;             // number of samples to retain (and calculate moving average over)
        uint32_t next_sample_;              // index of next free sample slot
        bool has_rolled_over_;

        float* samples_;
    };

}

#endif //WAVEGUIDE_SDR_FREQUENCYBIN_H
