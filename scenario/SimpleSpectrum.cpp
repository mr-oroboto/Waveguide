#include "SimpleSpectrum.h"

#include <iostream>

// Divide spectrum into this many regions, each of which can contain at most one interest marker.
#define INTEREST_MARKER_REGIONS 8

SimpleSpectrum::SimpleSpectrum(WindowManager *window_manager, sdr::SpectrumSampler *sampler, uint32_t bin_coalesce_factor)
        : insight::scenario::Scenario(window_manager->getDisplayManager()),
          window_manager_(window_manager), sampler_(sampler), bin_coalesce_factor_(bin_coalesce_factor)
{
    samples_ = sampler_->getSamples();

    bin_width_ = 0.5;

    max_freq_markers_ = 4;

    min_interest_marking_amplitude_ = 14.0f;
    max_interest_markers_ = INTEREST_MARKER_REGIONS;
    current_interest_markers_ = 0;

    set_initial_camera_ = false;
}

void SimpleSpectrum::resetState()
{
    frame_ = nullptr;

    // Sub-classes can register for mouse callbacks if they support them (ie. for zooming).
    window_manager_->setHandleMouseCallback(nullptr);
    samples_ = sampler_->getSamples();

    coalesced_bins_.clear();
    clearInterestMarkers();

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

    bin_coalesce_factor_ = coalesce_factor;
    clearInterestMarkers();

    std::cout << "Set bin coalesce factor to " << bin_coalesce_factor_ << std::endl;
}

uint64_t SimpleSpectrum::getMaxInterestMarkers()
{
    return max_interest_markers_;
}

void SimpleSpectrum::setMaxInterestMarkers(uint64_t max_interest_markers)
{
    if (max_interest_markers == 0)
    {
        return;
    }

    max_interest_markers_ = max_interest_markers;
    clearInterestMarkers();

    std::cout << "Set max interest markers to " << max_interest_markers_ << std::endl;
}

float SimpleSpectrum::getMinInterestMarkingAmplitude()
{
    return min_interest_marking_amplitude_;
}

void SimpleSpectrum::setMinInterestMarkingAmplitude(float min_amplitude)
{
    min_interest_marking_amplitude_ = min_amplitude;
    clearInterestMarkers();

    std::cout << "Set min interest marker amplitude to " << min_interest_marking_amplitude_ << std::endl;
}

void SimpleSpectrum::clearInterestMarkers()
{
    bin_ids_with_interest_markers_.clear();
    current_interest_markers_ = 0;
}

bool SimpleSpectrum::getBinHasInterestMarker(uint64_t bin_id)
{
    return bin_ids_with_interest_markers_.find(bin_id) != bin_ids_with_interest_markers_.end();
}

void SimpleSpectrum::setBinHasInterestMarker(uint64_t bin_id)
{
    if (current_interest_markers_ >= max_interest_markers_)
    {
        return;
    }

    // Don't put bin markers too close together
    uint64_t marking_range = coalesced_bins_.size() / max_interest_markers_;   // divide up total space into n regions, each one can only get 1 marker
    uint64_t start = (marking_range / 2) > bin_id ? 0 : bin_id - (marking_range / 2);
    uint64_t end = (bin_id + (marking_range / 2) >= coalesced_bins_.size()) ? coalesced_bins_.size() : bin_id + (marking_range / 2);

    for (uint64_t i = start; i < end; i++)
    {
        bin_ids_with_interest_markers_.insert(i);
    }

    current_interest_markers_++;
}

void SimpleSpectrum::markLocalMaxima()
{
    if (current_interest_markers_ >= max_interest_markers_)
    {
        return;
    }

    // Mark up local maxima as we haven't marked all our bins yet
    std::map<float, uint64_t> bin_amplitudes;
    for (SimpleSpectrumRange* bin : coalesced_bins_)
    {
        float amplitude = bin->getAmplitude();
        if (amplitude > min_interest_marking_amplitude_)
        {
            bin_amplitudes[amplitude] = bin->getBinId();
        }
    }

    for (std::map<float, uint64_t>::reverse_iterator i = bin_amplitudes.rbegin(); i != bin_amplitudes.rend(); i++)
    {
        if ( ! getBinHasInterestMarker(i->second))
        {
            SimpleSpectrumRange* bin = coalesced_bins_[i->second];
            addInterestMarkerToBin(bin);
        }
    }

}

void SimpleSpectrum::addInterestMarkerToBin(SimpleSpectrumRange *bin)
{
    setBinHasInterestMarker(bin->getBinId());
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

void SimpleSpectrum::highlightPickedBins(SimpleSpectrumRange *end_picking_bin)
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

