#include "SimpleSpectrum.h"

#define MARKER_REGIONS 8

SimpleSpectrum::SimpleSpectrum(DisplayManager *display_manager, sdr::SpectrumSampler *sampler, uint32_t bin_coalesce_factor)
        : Scenario(display_manager), sampler_(sampler), bin_coalesce_factor_(bin_coalesce_factor)
{
    assert(bin_coalesce_factor_ % 2 == 0);

    samples_ = sampler_->getSamples();
    bin_width_ = 0.5;

    max_markers_ = MARKER_REGIONS;
    current_markers_ = 0;

    set_initial_camera_ = false;
}

SimpleSpectrum::~SimpleSpectrum()
{
    std::cout << "SimpleSpectrum::~SimpleSpectrum()" << std::endl;
}

void SimpleSpectrum::resetState()
{
    coalesced_bins_.clear();
    marked_bins_.clear();

    current_markers_ = 0;

    set_initial_camera_ = false;
}

bool SimpleSpectrum::getBinHasBeenMarked(uint64_t bin_id)
{
    return marked_bins_.find(bin_id) != marked_bins_.end();
}

void SimpleSpectrum::setBinHasBeenMarked(uint64_t bin_id)
{
    if (current_markers_ >= max_markers_)
    {
        return;
    }

    // Don't put bin markers too close together
    uint64_t marking_range = coalesced_bins_.size() / max_markers_;   // divide up total space into n regions, each one can only get 1 marker
    uint64_t start = marking_range > bin_id ? 0 : bin_id - marking_range;
    uint64_t end = (bin_id + marking_range >= coalesced_bins_.size()) ? coalesced_bins_.size() : bin_id + marking_range;

    for (uint64_t i = start; i < end; i++)
    {
        marked_bins_.insert(i);
    }

    current_markers_++;
}

void SimpleSpectrum::markLocalMaxima()
{
    if (current_markers_ >= max_markers_)
    {
        return;
    }

    // Mark up local maxima as we haven't marked all our bins yet
    std::map<float, uint64_t> bin_amplitudes;
    for (SimpleSpectrumRange* bin : coalesced_bins_)
    {
        float amplitude = bin->getAmplitude();
        if (amplitude > 14.0f)
        {
            bin_amplitudes[amplitude] = bin->getBinId();
        }
    }

    for (std::map<float, uint64_t>::reverse_iterator i = bin_amplitudes.rbegin(); i != bin_amplitudes.rend(); i++)
    {
        if ( ! getBinHasBeenMarked(i->second))
        {
            SimpleSpectrumRange* bin = coalesced_bins_[i->second];
            markBin(bin);
        }
    }

}

void SimpleSpectrum::markBin(SimpleSpectrumRange* bin)
{
    setBinHasBeenMarked(bin->getBinId());
}

