#include "VectorSinkBlock.h"

#include <cmath>

sdr::VectorSinkBlock::VectorSinkBlock(std::string name, size_t vector_length, double bin_bw_hz, SpectrumSamples* samples) :
        gr::block(name, gr::io_signature::make(1, 1, sizeof(float) * vector_length), gr::io_signature::make(0, 0, 0)),
        vector_length_(vector_length), bin_bw_hz_(bin_bw_hz), samples_(samples)
{
    save_samples_ = false;
    sweep_count_ = 0;
}

sdr::VectorSinkBlock::~VectorSinkBlock()
{
}

sdr::VectorSinkBlock::sptr sdr::VectorSinkBlock::make(std::string block_name, size_t vector_length, double bin_bw_hz, SpectrumSamples* samples)
{
    return boost::shared_ptr<sdr::VectorSinkBlock>(new VectorSinkBlock(block_name, vector_length, bin_bw_hz, samples));
}

int sdr::VectorSinkBlock::general_work(int noutput_items, gr_vector_int &ninput_items,
                                       gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
{
    int vector_count = ninput_items[0];
    const float* vectors = static_cast<const float*>(input_items[0]);

    if (save_samples_)
    {
        for (int vector = 0; vector < vector_count; vector++)
        {
            const float* current_vector = vectors + (vector * vector_length_);
            updateSamples(current_vector);
        }
    }

    consume_each(vector_count);

    return 0;
}

void sdr::VectorSinkBlock::setCurrentFrequencyRange(uint64_t start_fft_freq_hz, uint64_t start_freq_hz, uint64_t end_freq_hz)
{
    start_fft_freq_hz_ = start_fft_freq_hz;
    start_freq_hz_ = start_freq_hz;
    end_freq_hz_ = end_freq_hz;

    setSaveSamples(true);
}

void sdr::VectorSinkBlock::setSaveSamples(bool save_samples)
{
    save_samples_ = save_samples;
}

void sdr::VectorSinkBlock::setSweepCount(uint64_t sweep_count)
{
    sweep_count_ = sweep_count;
}

void sdr::VectorSinkBlock::updateSamples(const float* scanned_amplitudes)
{
    for (size_t i = 0; i < vector_length_; i++)
    {
        float amplitude = scanned_amplitudes[i];

        // Convert the raw amplitude to a normalised amplitude
        //
        // TODO: Add an adjustment for the Blackman window
        // TODO: Normalise across all FFTs, not just this one

        float normalised_amplitude = (2.0f * sqrt(amplitude)) / vector_length_;
        float corrected_normalised_amplitude = 20.0f * (log(normalised_amplitude) / log(10.0f));

        uint64_t freq_hz = getBinFrequency(i);
        
        if (freq_hz >= start_freq_hz_ && freq_hz <= end_freq_hz_)
        {
            samples_->setLatestSample(freq_hz, corrected_normalised_amplitude, sweep_count_);
        }
        else if (freq_hz > end_freq_hz_)
        {
            break;
        }
    }
}

/**
 * The FFT bins are not linearly arranged, the center (tuned) frequency is in bin 0, the first half of the bins
 * contain the positive frequencies from the center and the second half contains the negative frequencies from
 * the center.
 */
uint64_t sdr::VectorSinkBlock::getBinFrequency(size_t bin_id)
{
    size_t translated_bin_id;

    if (bin_id < (vector_length_ / 2))
    {
        translated_bin_id = bin_id + (vector_length_ / 2);  // ie. 0 == 4096 (center frequency)
    }
    else
    {
        translated_bin_id = bin_id - (vector_length_ / 2);  // ie. 4096 == 0 (start of FFT)
    }

    return start_fft_freq_hz_ + static_cast<uint64_t>(translated_bin_id * bin_bw_hz_);
}