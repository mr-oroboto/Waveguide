#ifndef WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H
#define WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
#include "sdr/SpectrumSamples.h"

class LinearSpectrum : public Scenario
{
public:
    LinearSpectrum(DisplayManager* display_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~LinearSpectrum();

    void run();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);

    sdr::SpectrumSampler* sampler_;
    sdr::SpectrumSamples* samples_;
    uint32_t bin_coalesce_factor_;

    Frame* frame_;
};


#endif //WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H
