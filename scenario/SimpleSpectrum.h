#ifndef WAVEGUIDE_SCENARIO_SIMPLESPECTRUM_H
#define WAVEGUIDE_SCENARIO_SIMPLESPECTRUM_H

#include <vector>
#include <unordered_set>

#include <scenario/SimpleSpectrumRange.h>

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
#include "sdr/SpectrumSamples.h"

class SimpleSpectrum : public Scenario
{
public:
    SimpleSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    virtual ~SimpleSpectrum();

    uint32_t getCoalesceFactor();
    void setCoalesceFactor(uint32_t coalesce_factor);

protected:
    virtual void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame) = 0;

    void markLocalMaxima();
    bool getBinHasBeenMarked(uint64_t bin_id);
    void setBinHasBeenMarked(uint64_t bin_id);
    virtual void markBin(SimpleSpectrumRange* bin);

    void resetState();

    WindowManager* window_manager_;

    sdr::SpectrumSampler* sampler_;
    sdr::SpectrumSamples* samples_;

    bool set_initial_camera_;

    uint32_t bin_coalesce_factor_;
    float bin_width_;

    Frame* frame_;
    std::vector<SimpleSpectrumRange*> coalesced_bins_;

    uint64_t max_markers_;
    uint64_t current_markers_;
    std::unordered_set<uint64_t> marked_bins_;
};

#endif //WAVEGUIDE_SCENARIO_SIMPLESPECTRUM_H
