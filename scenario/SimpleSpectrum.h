#ifndef WAVEGUIDE_SCENARIO_SIMPLESPECTRUM_H
#define WAVEGUIDE_SCENARIO_SIMPLESPECTRUM_H

#include <vector>
#include <unordered_set>
#include <stack>

#include <scenario/SimpleSpectrumRange.h>

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
#include "sdr/SpectrumSamples.h"

// Base class from which all spectrum display scenarios inherit.
class SimpleSpectrum : public Scenario
{
public:
    SimpleSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    virtual ~SimpleSpectrum();

    // Get and set the coalesce factor for the underlying sdr::FrequencyBin instances (see bin_coalesce_factor_ below).
    uint32_t getCoalesceFactor();
    void setCoalesceFactor(uint32_t coalesce_factor);

    // Get and set the maximum number of interest markers that can be placed along the spectrum.
    uint64_t getMaxInterestMarkers();
    void setMaxInterestMarkers(uint64_t max_interest_markers);

    // Get and set the the minimum amplitude a SimpleSpectrumRange must have before it's considered "of interest".
    float getMinInterestMarkingAmplitude();
    void setMinInterestMarkingAmplitude(float min_amplitude);

    // Remove interest markers from any marked bins.
    virtual void clearInterestMarkers();

    // Undo the previous zoom-in by popping a ZoomRange off the stack.
    virtual void undoLastZoom();

    // Adjust the frequency range being scanned, used when zooming in and out.
    void retune(uint64_t start_freq_hz, uint64_t end_freq_hz, bool zooming_in);

protected:
    // The specific range the sampler_ covers is modelled by a ZoomRange.
    typedef struct
    {
        uint64_t start_freq_hz_;
        uint64_t end_freq_hz_;
    } ZoomRange;

    // Called when redrawing the scene.
    virtual void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame) = 0;

    // Handle mouse callbacks (ie. notification of mouse down, click, mouse up etc).
    virtual bool handleMouse(WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop);

    // Called when updating the scene and fewer than max_interest_markers_ frequencies of interest have been marked.
    // Finds all SimpleSpectrumRange instances whose amplitude is greater than min_interest_marking_amplitude_ and
    // marks those with the highest amplitude by using addInterestMarkerToBin().
    void markLocalMaxima();

    // Mark a coalesced frequency bin (using whatever technique is best for the scenario, could be simple text, could
    // be an arrow etc) as being "of interest" (generally because it has high amplitude).
    virtual void addInterestMarkerToBin(SimpleSpectrumRange *bin);

    // Get / set whether a specific SimpleSpectrumRange has an interest marker.
    bool getBinHasInterestMarker(uint64_t bin_id);
    void setBinHasInterestMarker(uint64_t bin_id);

    // Highlight the SimpleSpectrumRanges that are "picked" when ray casting for a zoom.
    virtual void highlightPickedBins(SimpleSpectrumRange *end_picking_bin);

    // Called by sub-classes when the Scenario is run() by ScenarioCollection.
    void resetState();

    WindowManager* window_manager_;
    Frame* frame_;
    bool set_initial_camera_;

    // Interface to SDR hardware.
    sdr::SpectrumSampler* sampler_;
    sdr::SpectrumSamples* samples_;

    // The current range being scanned (from start_freq_hz to end_freq_hz) is split into n slices, where each slice is
    // the bandwidth of the capture device. sdr::SampleThread dwells on each slice for a period of time over which it
    // repeatedly performs FFTs on the slice of spectrum. The FFT has a certain number of bins, each represented by an
    // sdr::FrequencyBin that covers a small amount of spectrum (ie. generally around 2000Hz).
    //
    // Rather than render each sdr::FrequencyBin as a unique object in the scene, the bins are coalesced together which
    // averages the amplitude of the individual bins into a single value that controls the size of the scene object. A
    // small bin_coalesce_factor_ means fewer bins are grouped together, resulting in more objects on screen (and more
    // fidelity), but slower rendering. A large bin_coalesce_factor_ means more bins are grouped together, which will
    // increase the frame rate but smear small signals together (until they're ultimately lost).
    //
    // This factor can be controlled at runtime by the user so that they can drill into the detail when needed (which
    // is different to zooming, which focuses the scanning range on a smaller portion of the spectrum).
    uint32_t bin_coalesce_factor_;

    // Collection of SceneObjects, each of which represents a group of coalesced sdr::FrequencyBins.
    std::vector<SimpleSpectrumRange*> coalesced_bins_;

    // The width of the scene object that represents a coalesced frequency bin. Smaller means more bins can be fit
    // on the screen but individual amplitude spikes may be harder to see. Intepretation of the value really depends
    // on what type of object (cube, rectangle, line) the scenario subclass uses.
    float bin_width_;

    // Max number of regular frequency markers the scenario can place along the frequency dimension.
    uint64_t max_freq_markers_;

    // Only SimpleSpectrumRanges with a combined amplitude greater than this are considered by markLocalMaxima().
    float min_interest_marking_amplitude_;

    // Max and current number of SimpleSpectrumRanges to place "interest markers" on.
    uint64_t max_interest_markers_;
    uint64_t current_interest_markers_;

    // IDs of the SimpleSpectrumRanges that currently have interest markers.
    std::unordered_set<uint64_t> bin_ids_with_interest_markers_;

    SimpleSpectrumRange* start_picking_bin_;
    SimpleSpectrumRange* last_picked_bin_;

    // Zooming into a new range pushes the current ZoomRange onto the stack.
    std::stack<ZoomRange> previous_zoom_ranges_;
};

#endif //WAVEGUIDE_SCENARIO_SIMPLESPECTRUM_H
