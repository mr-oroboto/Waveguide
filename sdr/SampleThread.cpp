#include "SampleThread.h"

#include "VectorSinkBlock.h"

#include <iostream>
#include <vector>
#include <cmath>

#include <unistd.h>

#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source_c.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/probe_signal_vf.h>
#include <gnuradio/fft/fft_vcc.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/blocks/null_sink.h>
#include <osmosdr/source.h>

#include "Config.h"

sdr::SampleThread::SampleThread(Config* config, uint8_t device_id, uint64_t start_freq_hz,
                                uint64_t end_freq_hz, SpectrumSamples* samples) :
    config_(config), device_id_(device_id), start_freq_hz_(start_freq_hz), end_freq_hz_(end_freq_hz),
    samples_(samples)
{
    device_type_ = config->getDevicePrefix();
    sample_rate_hz_ = config->getSampleRate();

    stop_ = false;
    thread_ = nullptr;
    sweep_count_ = 0;

    dwell_time_us_ = config->getDwellTime();
}

sdr::SampleThread::~SampleThread()
{
}

bool sdr::SampleThread::start()
{
    if (thread_)
    {
        std::cout << "Sample thread on " << start_freq_hz_ << "Hz is already running" << std::endl;
        return false;
    }

    thread_ = new std::thread(std::ref(*this));

    return true;
}

bool sdr::SampleThread::stop()
{
    bool stopped = false;

    if (thread_)
    {
        std::cout << "Signalling sample thread on " << start_freq_hz_ << "Hz to stop" << std::endl;

        stop_ = true;

        thread_->join();

        delete thread_;
        thread_ = nullptr;

        stopped = true;
    }

    return stopped;
}

void sdr::SampleThread::operator()()
{
    std::cout << "Starting sample thread on " << start_freq_hz_ << "Hz (sample rate: " << sample_rate_hz_ << "Hz, gain: " << config_->getGain() << "dB)" << std::endl;

    size_t vector_length = samples_->getFFTSize();
    uint64_t total_bw_hz = (end_freq_hz_ - start_freq_hz_) + 1;

    gr::top_block_sptr top_block;
    osmosdr::source::sptr hardware_src;
    gr::blocks::stream_to_vector::sptr stream_to_vec;
    gr::fft::fft_vcc::sptr fft;
    gr::blocks::complex_to_mag_squared::sptr complex_to_mag2;
    VectorSinkBlock::sptr vector_sink;

    std::vector<float> blackman_window = gr::filter::firdes::window(gr::filter::firdes::WIN_BLACKMAN_HARRIS, vector_length /* # taps */, 6.67);

    char top_block_name[64], vector_sink_name[64];
    snprintf(top_block_name, sizeof(top_block_name), "spectrum%lu", start_freq_hz_);
    snprintf(vector_sink_name, sizeof(vector_sink_name), "vector_sink%lu", start_freq_hz_);

    top_block = gr::make_top_block(top_block_name);

    char hardware_src_name[64];
    snprintf(hardware_src_name, sizeof(hardware_src_name), "%s=%u", device_type_.c_str(), device_id_);
    hardware_src = osmosdr::source::make(hardware_src_name);

    hardware_src->set_sample_rate(sample_rate_hz_);
    hardware_src->set_center_freq(start_freq_hz_);
    hardware_src->set_gain_mode(false);     // disable AGC
    hardware_src->set_gain(config_->getGain());
//  hardware_src->set_if_gain(20);

    stream_to_vec = gr::blocks::stream_to_vector::make(sizeof(gr_complex), vector_length);
    fft = gr::fft::fft_vcc::make(vector_length, true, blackman_window, true);
    complex_to_mag2 = gr::blocks::complex_to_mag_squared::make(vector_length);
    vector_sink = VectorSinkBlock::make(vector_sink_name, vector_length, samples_->getBinBandwidth(), samples_);

    top_block->connect(hardware_src, 0, stream_to_vec, 0);
    top_block->connect(stream_to_vec, 0, fft, 0);
    top_block->connect(fft, 0, complex_to_mag2, 0);
    top_block->connect(complex_to_mag2, 0, vector_sink, 0);

    top_block->start();

    uint64_t tune_freq_hz = start_freq_hz_;
    uint32_t slice_id = 0;
    bool retune = true;

    while ( ! stop_)
    {
        if (retune)
        {
            // The FFT straddles the center tuning frequency
            uint64_t start_fft_freq_hz = static_cast<uint64_t>(tune_freq_hz - (sample_rate_hz_ / 2.0));  // TODO: watch for underrun
            uint64_t end_fft_freq_hz = static_cast<uint64_t>(tune_freq_hz + (sample_rate_hz_ / 2.0));

            // But the range we're scanning is sometimes smaller (at the first and last slice)
            uint64_t start_slice_freq_hz = start_fft_freq_hz >= start_freq_hz_ ? start_fft_freq_hz : start_freq_hz_;
            uint64_t end_slice_freq_hz = end_fft_freq_hz <= end_freq_hz_ ? end_fft_freq_hz : end_freq_hz_;

//            std::cout << "Slice: " << slice_id << ", tuned to " << tune_freq_hz << "Hz (FFT: " << start_fft_freq_hz << ", " << end_fft_freq_hz << ") (Slice: " << start_slice_freq_hz << ", " << end_slice_freq_hz << ")" << std::endl;
            double tuned_freq_hz = hardware_src->set_center_freq(tune_freq_hz);
//            usleep(100000);                                         // is this required?

            vector_sink->setCurrentFrequencyRange(start_fft_freq_hz, start_slice_freq_hz, end_slice_freq_hz);
            last_retuned_at_ = std::chrono::high_resolution_clock::now();

            retune = false;
            slice_id++;
        }

        // If our dwell time has elapsed, it's time to retune
        auto t_now = std::chrono::high_resolution_clock::now();
        float secs_since_last_retune = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - last_retuned_at_).count();

        if ((secs_since_last_retune * 1000000) > dwell_time_us_)
        {
            vector_sink->setSaveSamples(false);             // don't update data while retuning

            tune_freq_hz += sample_rate_hz_ / 2.0;
            retune = true;

            if ((tune_freq_hz - (sample_rate_hz_ / 2.0)) > end_freq_hz_)
            {
                tune_freq_hz = start_freq_hz_;
                slice_id = 0;
                sweep_count_++;

                vector_sink->setSweepCount(sweep_count_);
            }
        }
    }

    top_block->stop();
    top_block->wait();

    std::cout << "Sample thread on " << start_freq_hz_ << "Hz is exiting" << std::endl;
}


