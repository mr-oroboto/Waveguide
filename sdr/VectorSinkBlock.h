#ifndef WAVEGUIDE_SDR_VECTORSINKBLOCK_H
#define WAVEGUIDE_SDR_VECTORSINKBLOCK_H

#include <string>

#include <gnuradio/block.h>

#include "SpectrumSamples.h"

namespace sdr {

    class VectorSinkBlock : public gr::block {
    public:
        VectorSinkBlock(std::string block_name, size_t vector_length, double bin_bw_hz, SpectrumSamples* samples);
        virtual ~VectorSinkBlock();

        typedef boost::shared_ptr<VectorSinkBlock> sptr;

        static sptr make(std::string block_name, size_t vector_length, double bin_bw_hz, SpectrumSamples* samples);

        void setCurrentFrequencyRange(uint64_t start_fft_freq_hz, uint64_t start_freq_hz, uint64_t end_freq_hz);

        void setSaveSamples(bool save_samples);
        void setSweepCount(uint64_t sweep_count);

    private:
        virtual int general_work(int noutput_items, gr_vector_int &ninput_items, gr_vector_const_void_star &input_items,
                                 gr_vector_void_star &output_items);

        void updateSamples(const float *scanned_amplitudes);
        uint64_t getBinFrequency(size_t bin_id);

        size_t vector_length_;

        SpectrumSamples *samples_;
        bool save_samples_;                 // samples should be actively saved when received

        uint64_t start_fft_freq_hz_;        // the FFT runs from this frequency to this + sample rate
        uint64_t start_freq_hz_;            // sample bins at or past this frequency
        uint64_t end_freq_hz_;              // don't sample bins past this frequency
        double bin_bw_hz_;                  // each bin is this wide

        uint64_t sweep_count_;              // tracks value from SampleThread
    };
}

#endif //WAVEGUIDE_SDR_VECTORSINKBLOCK_H
