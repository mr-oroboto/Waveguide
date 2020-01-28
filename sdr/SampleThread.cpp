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

sdr::SampleThread::SampleThread(const std::string& device_type, uint8_t device_id, uint64_t start_freq_hz,
                                uint64_t end_freq_hz, uint64_t sample_rate_hz, SpectrumSamples* samples) :
    device_type_(device_type), device_id_(device_id), start_freq_hz_(start_freq_hz), end_freq_hz_(end_freq_hz),
    sample_rate_hz_(sample_rate_hz), samples_(samples)
{
    stop_ = false;
    thread_ = nullptr;
    sweep_count_ = 0;

    dwell_time_us_ = 500000;
}

sdr::SampleThread::~SampleThread()
{
    std::cout << "SampleThread::~SampleThread()" << std::endl;
}

bool sdr::SampleThread::start()
{
    if (thread_)
    {
        std::cout << "Sample thread on " << start_freq_hz_ << "Hz is already running" << std::endl;
        return false;
    }

    thread_ = new std::thread(std::ref(*this));

    std::cout << "Sample thread on " << start_freq_hz_ << "Hz has started" << std::endl;

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

        std::cout << "Sample thread on " << start_freq_hz_ << "Hz has stopped" << std::endl;

        delete thread_;
        thread_ = nullptr;

        stopped = true;
    }

    return stopped;
}

void sdr::SampleThread::operator()()
{
    std::cout << "Starting sample thread on " << start_freq_hz_ << "Hz" << std::endl;

    size_t vector_length = samples_->getFFTSize();
    uint64_t total_bw_hz = end_freq_hz_ - start_freq_hz_;

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
//  hardware_src->set_bandwidth();
    hardware_src->set_gain_mode(false);     // disable AGC
    hardware_src->set_gain(41.6);
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

    uint64_t slices = static_cast<uint64_t>(ceil(total_bw_hz / static_cast<long double>(sample_rate_hz_)));
    std::cout << "Dividing total bandwidth of " << total_bw_hz << "Hz into " << slices << " slices (sample rate: " << sample_rate_hz_ << "Hz)" << std::endl;

    uint64_t start_slice_freq_hz = start_freq_hz_;

    bool even_pass = true;
    bool retune = true;

    while ( ! stop_)
    {
        if (retune)
        {
            uint64_t tune_freq_hz, end_freq_hz, start_iteration_freq_hz;

            if (even_pass)
            {
                // Tune to the center of the slice
                tune_freq_hz = static_cast<uint64_t>(start_slice_freq_hz + (sample_rate_hz_ / 2.0));
                end_freq_hz = (end_freq_hz_ < (start_slice_freq_hz + sample_rate_hz_)) ? end_freq_hz_ : (start_slice_freq_hz + sample_rate_hz_);
                start_iteration_freq_hz = start_slice_freq_hz;
            }
            else
            {
                // Tune to the start of the slice
                tune_freq_hz = start_slice_freq_hz;
                end_freq_hz = static_cast<uint64_t>((end_freq_hz_ < (start_slice_freq_hz + (sample_rate_hz_ / 2.0))) ? end_freq_hz_ : (start_slice_freq_hz + (sample_rate_hz_ / 2.0)));
                start_iteration_freq_hz = static_cast<uint64_t>(start_slice_freq_hz - (sample_rate_hz_ / 2.0));     // todo: watch for underrun
            }

            double tuned_freq_hz = hardware_src->set_center_freq(tune_freq_hz);
            usleep(100000);                                         // is this required?

            vector_sink->setCurrentFrequencyRange(start_iteration_freq_hz, start_slice_freq_hz, end_freq_hz);
            last_retuned_at_ = std::chrono::high_resolution_clock::now();

            retune = false;

            std::cout << "Tuned to " << tuned_freq_hz << "Hz (requested: " << tune_freq_hz << "Hz) for " << (even_pass ? "even" : "odd") << " scan from " << start_iteration_freq_hz << "Hz to " << end_freq_hz << "Hz (slice starts at " << start_slice_freq_hz << "Hz)" << std::endl;
        }

        // If our dwell time has elapsed, it's time to retune
        auto t_now = std::chrono::high_resolution_clock::now();
        float secs_since_last_retune = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - last_retuned_at_).count();

        if ((secs_since_last_retune * 1000000) > dwell_time_us_)
        {
            vector_sink->setSaveSamples(false);             // don't update data while retuning

            start_slice_freq_hz += sample_rate_hz_;
            retune = true;

            if (start_slice_freq_hz > end_freq_hz_)
            {
                start_slice_freq_hz = start_freq_hz_;
                even_pass = ! even_pass;
                sweep_count_++;

                vector_sink->setSweepCount(sweep_count_);
            }
        }
    }

    std::cout << "Sample thread on " << start_freq_hz_ << "Hz is stopping flowgraph" << std::endl;

    top_block->stop();

    std::cout << "Sample thread on " << start_freq_hz_ << "Hz is exiting" << std::endl;
}


