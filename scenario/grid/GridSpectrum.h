#ifndef WAVEGUIDE_SCENARIO_GRID_GRIDSPECTRUM_H
#define WAVEGUIDE_SCENARIO_GRID_GRIDSPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class GridSpectrum : public SimpleSpectrum
{
public:
    GridSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor);
    ~GridSpectrum();

    void run();
    void clearMarkedBins();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);
    void markBin(SimpleSpectrumRange* bin);

    std::vector<unsigned long> marked_bin_text_ids_;
};


#endif //WAVEGUIDE_SCENARIO_GRID_GRIDSPECTRUM_H
