#include "FrequencyBin.h"

#include <iostream>
#include <cstring>
#include <cassert>

sdr::FrequencyBin::FrequencyBin(uint64_t freq_hz, uint16_t history_size) : freq_hz_(freq_hz), history_size_(history_size)
{
    max_amplitude_ = 0.0f;
    moving_average_amplitude_ = 0.0f;

    next_sample_ = 0;
    has_rolled_over_ = false;

    samples_ = new float[history_size_];
    memset(samples_, 0, sizeof(float) * history_size_);
}

sdr::FrequencyBin::~FrequencyBin()
{
    delete[] samples_;
}

uint64_t sdr::FrequencyBin::getFrequency()
{
    return freq_hz_;
}

bool sdr::FrequencyBin::getHasBeenSet()
{
    // Lock the sample data so that others don't read it from under us
    std::lock_guard<std::mutex> guard(lock_);

    return ! ( ! has_rolled_over_ && next_sample_ == 0);
}

float sdr::FrequencyBin::getLatestAmplitude(bool moving_average)
{
    // Lock the sample metadata so that the current_sample_ doesn't change under our feet
    std::lock_guard<std::mutex> guard(lock_);

    uint16_t current_sample = 0;
    if (has_rolled_over_)
    {
        current_sample = (next_sample_ == 0) ? history_size_ - 1 : next_sample_ - 1;
    }
    else
    {
        current_sample = (next_sample_ == 0) ? 0 : next_sample_ - 1;
    }

    assert(current_sample < history_size_);

    return moving_average ? moving_average_amplitude_ : samples_[current_sample];
}

float sdr::FrequencyBin::getMaximumAmplitude()
{
    std::lock_guard<std::mutex> guard(lock_);

    return max_amplitude_;
}

void sdr::FrequencyBin::setLatestAmplitude(float amplitude, bool keep_maximum)
{
    // Lock the sample data so that others don't read it from under us
    std::lock_guard<std::mutex> guard(lock_);

    uint32_t current_sample = next_sample_;

    assert(current_sample < history_size_);
    samples_[current_sample] = amplitude;

    // Keep the maximum amplitude seen for this frequency
    if (keep_maximum && ((current_sample == 0 && ! has_rolled_over_) || (amplitude > max_amplitude_)))
    {
        max_amplitude_ = amplitude;
    }

    // Calculate the moving average
    float divisor = 1.0f;
    float previous_amplitude = 0.0f;

    if (has_rolled_over_)
    {
        divisor = history_size_;
        previous_amplitude = (current_sample == 0) ? (samples_[history_size_ - 1]) : (samples_[current_sample - 1]);
    }
    else
    {
        divisor = current_sample + 1.0f;
        previous_amplitude = (current_sample == 0) ? 0.0f : samples_[current_sample - 1];
    }

    moving_average_amplitude_ += ((1.0f / divisor) * (amplitude - previous_amplitude));

    if (++next_sample_ >= history_size_)
    {
        next_sample_ = 0;
        has_rolled_over_ = true;
    }
}