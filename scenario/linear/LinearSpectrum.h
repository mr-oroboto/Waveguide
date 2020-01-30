#ifndef WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H
#define WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class LinearSpectrum : public SimpleSpectrum
{
public:
    LinearSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~LinearSpectrum();

    void run();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);
    void markBin(SimpleSpectrumRange* bin);
};


#endif //WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H
