#include "SimpleSpectrum.h"

#define MARKER_REGIONS 8

SimpleSpectrum::SimpleSpectrum(WindowManager *window_manager, sdr::SpectrumSampler *sampler, uint32_t bin_coalesce_factor)
        : Scenario(window_manager->getDisplayManager()),
          window_manager_(window_manager), sampler_(sampler), bin_coalesce_factor_(bin_coalesce_factor)
{
    samples_ = sampler_->getSamples();
    bin_width_ = 0.5;

    max_freq_markers_ = 4;
    max_markers_ = MARKER_REGIONS;
    current_markers_ = 0;

    set_initial_camera_ = false;
}

SimpleSpectrum::~SimpleSpectrum()
{
}

void SimpleSpectrum::resetState()
{
    frame_ = nullptr;
    
    window_manager_->setHandleMouseCallback(nullptr);
    samples_ = sampler_->getSamples();

    coalesced_bins_.clear();
    clearMarkedBins();

    set_initial_camera_ = false;
}

uint32_t SimpleSpectrum::getCoalesceFactor()
{
    return bin_coalesce_factor_;
}

void SimpleSpectrum::setCoalesceFactor(uint32_t coalesce_factor)
{
    if (coalesce_factor == 0)
    {
        return;
    }

    std::cout << "Set bin coalesce factor to " << coalesce_factor << std::endl;

    bin_coalesce_factor_ = coalesce_factor;

    clearMarkedBins();
}

void SimpleSpectrum::clearMarkedBins()
{
    marked_bins_.clear();
    current_markers_ = 0;
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

bool SimpleSpectrum::handleMouse(WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop)
{
    return true;
}

void SimpleSpectrum::undoLastZoom()
{
    if (previous_zoom_ranges_.size())
    {
        ZoomRange previous_range = previous_zoom_ranges_.top();
        previous_zoom_ranges_.pop();

        retune(previous_range.start_freq_hz_, previous_range.end_freq_hz_, false);
    }
}

void SimpleSpectrum::retune(uint64_t start_freq_hz, uint64_t end_freq_hz, bool zooming_in)
{
    // Retune from the frequency of start_picking_bin_ to last_picked_bin_
    sampler_->stop();           // this is blocking and invalidates samples_

    sampler_->start(start_freq_hz, end_freq_hz);

    // Now that we've retuned we have a new frequency range
    samples_ = sampler_->getSamples();

    if (zooming_in)
    {
        bin_coalesce_factor_ /= 2.0f;
        if (bin_coalesce_factor_ == 0)
        {
            bin_coalesce_factor_ = 1;
        }
    }
    else
    {
        bin_coalesce_factor_ *= 2.0f;
    }

    // Set up the new frame
    run();
}

void SimpleSpectrum::markPickedBins(SimpleSpectrumRange* end_picking_bin)
{
    if (end_picking_bin == start_picking_bin_)
    {
        end_picking_bin->setPicked(true);
        return;
    }

    bool picking_right = end_picking_bin->getBinId() > start_picking_bin_->getBinId();
    SimpleSpectrumRange* start = picking_right ? start_picking_bin_ : end_picking_bin;
    SimpleSpectrumRange* end = picking_right ? end_picking_bin : start_picking_bin_;

    for (uint64_t bin_id = start->getBinId(); picking_right ? bin_id <= end->getBinId() && bin_id < coalesced_bins_.size() : bin_id >= start->getBinId() && bin_id > 0; picking_right ? bin_id++ : bin_id--)
    {
        last_picked_bin_ = coalesced_bins_[bin_id];
        last_picked_bin_->setPicked(true);
    }

    end->setPicked(true);
}

