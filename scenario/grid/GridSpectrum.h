#ifndef WAVEGUIDE_SCENARIO_GRID_GRIDSPECTRUM_H
#define WAVEGUIDE_SCENARIO_GRID_GRIDSPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class GridSpectrum : public SimpleSpectrum
{
public:
    GridSpectrum(DisplayManager* display_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor);
    ~GridSpectrum();

    void run();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);
    void markBin(SimpleSpectrumRange* bin);
};


#endif //WAVEGUIDE_SCENARIO_GRID_GRIDSPECTRUM_H
