#ifndef WAVEGUIDE_SCENARIO_LINEAR_LINEARTIMESPECTRUM_H
#define WAVEGUIDE_SCENARIO_LINEAR_LINEARTIMESPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class LinearTimeSpectrum : public SimpleSpectrum
{
public:
    LinearTimeSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~LinearTimeSpectrum();

    void run();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);
    void markBin(SimpleSpectrumRange* bin);

    void addSpectrumRanges(uint16_t slice_id, GLfloat secs_since_framequeue_started);

    uint16_t slices_;
    uint16_t current_slice_;
    uint64_t current_sweep_;
};


#endif //WAVEGUIDE_SCENARIO_LINEAR_LINEARTIMESPECTRUM_H
