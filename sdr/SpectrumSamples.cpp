#include "SpectrumSamples.h"

#include <iostream>
#include <cassert>
#include <cmath>

#define FFT_SIZE 8192

sdr::SpectrumSamples::SpectrumSamples(uint64_t start_freq_hz, uint64_t end_freq_hz, uint64_t capture_sample_rate_hz, uint16_t history_size) :
        start_freq_hz_(start_freq_hz), end_freq_hz_(end_freq_hz), capture_sample_rate_hz_(capture_sample_rate_hz)
{
    fft_size_ = FFT_SIZE;

    keep_maximum_sample_ = true;
    sweep_count_ = 0;

    assert(end_freq_hz_ > start_freq_hz_);

    uint64_t total_bw_hz = (end_freq_hz_ - start_freq_hz_) + 1;     // inclusive of start and end (ie. 1000 - 1 = 1000Hz)

    // How much bandwidth is represented by an FFT bin and how many total bins are required for the whole range?
    bin_bw_hz_ = floor(capture_sample_rate_hz_ / static_cast<double>(fft_size_));
    uint64_t bin_count = static_cast<uint64_t>(ceil(total_bw_hz / bin_bw_hz_));
    std::cout << "Allocating " << bin_count << " bins (" << bin_bw_hz_ << "Hz per bin) to cover " << total_bw_hz << "Hz" << std::endl;

    uint64_t bin_start_freq_hz = start_freq_hz_;
    for (uint64_t i = 0; i < bin_count; i++)
    {
        bins_.push_back(new FrequencyBin(bin_start_freq_hz, history_size));
        bin_start_freq_hz += bin_bw_hz_;
    }
}

sdr::SpectrumSamples::~SpectrumSamples()
{
    for (uint64_t i = 0; i < bins_.size(); i++)
    {
        delete bins_[i];
    }
}

uint32_t sdr::SpectrumSamples::getFFTSize()
{
    return fft_size_;
}

double sdr::SpectrumSamples::getBinBandwidth()
{
    return bin_bw_hz_;
}

float sdr::SpectrumSamples::getLatestAmplitude(uint64_t freq_hz, bool moving_average)
{
    return bins_[getBinNumber(freq_hz)]->getLatestAmplitude(moving_average);
}

sdr::FrequencyBin const* sdr::SpectrumSamples::getFrequencyBin(uint64_t bin_number)
{
    assert(bin_number < bins_.size());
    return bins_[bin_number];
}

uint64_t sdr::SpectrumSamples::getBinCount()
{
    return bins_.size();
}

void sdr::SpectrumSamples::setKeepMaximumSample(bool keep_maximum_sample)
{
    keep_maximum_sample_ = keep_maximum_sample;
}

void sdr::SpectrumSamples::setLatestSample(uint64_t freq_hz, float amplitude, uint64_t sweep_count)
{
    uint64_t bin_number = getBinNumber(freq_hz);

    bins_[bin_number]->setLatestAmplitude(amplitude, keep_maximum_sample_);

    // If any of the sampler threads has moved onto its next sweep, keep our sweep count aligned
    if (sweep_count > sweep_count_)
    {
        sweep_count_ = sweep_count;
    }
}

uint64_t sdr::SpectrumSamples::getSweepCount()
{
    return sweep_count_;
}

uint64_t sdr::SpectrumSamples::getBinNumber(uint64_t freq_hz)
{
    uint64_t freq_offset_hz = freq_hz - start_freq_hz_;

    assert(freq_offset_hz >= 0);

    uint64_t bin_number = static_cast<uint64_t>(floor(freq_offset_hz / bin_bw_hz_));

    assert(bin_number < bins_.size());

    return bin_number;
}

