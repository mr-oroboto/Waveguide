#ifndef WAVEGUIDE_SCENARIO_SPHERE_SPHERESPECTRUM_H
#define WAVEGUIDE_SCENARIO_SPHERE_SPHERESPECTRUM_H

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
#include "sdr/SpectrumSamples.h"

class SphereSpectrum : public Scenario
{
public:
    SphereSpectrum(DisplayManager* display_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~SphereSpectrum();

    void run();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);

    sdr::SpectrumSampler* sampler_;
    sdr::SpectrumSamples* samples_;
    uint32_t bin_coalesce_factor_;
    uint16_t radius_;

    uint16_t rings_;
    uint16_t current_ring_;
    uint64_t current_sweep_;

    Frame* frame_;
};


#endif //WAVEGUIDE_SCENARIO_SPHERE_SPHERESPECTRUM_H
