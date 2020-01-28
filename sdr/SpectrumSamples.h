#ifndef WAVEGUIDE_SDR_SPECTRUMSAMPLES_H
#define WAVEGUIDE_SDR_SPECTRUMSAMPLES_H

#include <vector>
#include <cstdint>

#include "FrequencyBin.h"

namespace sdr {

    class SampleThread;
    class FrequencyBin;

    class SpectrumSamples {
    public:
        SpectrumSamples(uint64_t start_freq_hz, uint64_t end_freq_hz, uint64_t capture_sample_rate_hz);
        ~SpectrumSamples();

        float getLatestAmplitude(uint64_t freq_hz, bool moving_average = true);

        uint64_t getBinCount();
        FrequencyBin const* getFrequencyBin(uint64_t bin_number);

        void setKeepMaximumSample(bool keep_maximum_sample);

        uint32_t getFFTSize();
        double getBinBandwidth();

        uint64_t getSweepCount();

    private:
        friend class VectorSinkBlock;

        void setLatestSample(uint64_t freq_hz, float amplitude, uint64_t sweep_count);
        uint64_t getBinNumber(uint64_t freq_hz);

        bool keep_maximum_sample_;          // if keeping a single sample, do we keep the latest or the max?

        uint64_t start_freq_hz_;            // starting frequency for samples
        uint64_t end_freq_hz_;              // end frequency for samples
        uint64_t capture_sample_rate_hz_;   // sample rate of the capture device(s)
        double bin_bw_hz_;                  // bandwidth of each frequency bin
        uint64_t sweep_count_;              // how many sweeps of the full spectrum have been performed by the sampler threads?

        uint32_t fft_size_;

        std::vector<FrequencyBin*> bins_;
    };

}

#endif //WAVEGUIDE_SDR_SPECTRUMSAMPLES_H
