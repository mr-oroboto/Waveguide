#include "SpectrumSampler.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <boost/thread/exceptions.hpp>

#include "Config.h"

sdr::SpectrumSampler::SpectrumSampler(Config* config) :
    config_(config)
{
    device_count_ = config->getDeviceCount();
    capture_device_sample_rate_hz_ = config->getSampleRate();

    assert(device_count_ >= 1);

    sample_threads_.clear();
    samples_ = nullptr;
}

sdr::SpectrumSampler::~SpectrumSampler()
{
    stop();
}

sdr::SpectrumSamples* sdr::SpectrumSampler::getSamples()
{
    return samples_;
}

void sdr::SpectrumSampler::stop()
{
    std::cout << "Signalling all sample threads to exit" << std::endl;

    for (SampleThread* t : sample_threads_)
    {
        // This will block until the thread has stopped
        t->stop();
        delete t;
    }

    sample_threads_.clear();

    if (samples_)
    {
        delete samples_;
        samples_ = nullptr;
    }

    std::cout << "All sample threads have been stopped" << std::endl;
}

bool sdr::SpectrumSampler::start(uint64_t start_freq_hz, uint64_t end_freq_hz)
{
    if (sample_threads_.size())
    {
        std::cout << "Cannot start SpectrumSampler while it is already running, call stop() first" << std::endl;
        return false;
    }

    start_freq_hz_ = start_freq_hz;
    end_freq_hz_ = end_freq_hz;

    samples_ = new SpectrumSamples(start_freq_hz, end_freq_hz, capture_device_sample_rate_hz_, config_->getAveragingWindow());

    uint64_t total_bw_hz = end_freq_hz - start_freq_hz;
    uint64_t bw_per_device_hz = static_cast<uint64_t>(ceil(total_bw_hz / static_cast<float>(device_count_)));        // may be > capture_device_sample_rate_hz_
    uint64_t device_start_freq_hz = start_freq_hz;

    for (uint8_t i = 0; i < device_count_; i++)
    {
        SampleThread* thread = new SampleThread(config_, i, device_start_freq_hz, device_start_freq_hz + bw_per_device_hz, samples_);
        sample_threads_.push_back(thread);

        thread->start();

        device_start_freq_hz += bw_per_device_hz;
    }

    return true;
}

uint64_t sdr::SpectrumSampler::getStartFrequency()
{
    return start_freq_hz_;
}

uint64_t sdr::SpectrumSampler::getEndFrequency()
{
    return end_freq_hz_;
}