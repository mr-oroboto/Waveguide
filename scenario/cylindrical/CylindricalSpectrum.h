#ifndef WAVEGUIDE_SCENARIO_CYLINDRICAL_CYLINDRICALSPECTRUM_H
#define WAVEGUIDE_SCENARIO_CYLINDRICAL_CYLINDRICALSPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class CylindricalSpectrum : public SimpleSpectrum
{
public:
    CylindricalSpectrum(DisplayManager* display_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~CylindricalSpectrum();

    void run();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);

    void addSpectrumRanges(uint16_t ring_id, GLfloat secs_since_framequeue_started);

    uint16_t radius_;

    uint16_t rings_;
    uint16_t current_ring_;
    uint64_t current_sweep_;
};


#endif //WAVEGUIDE_SCENARIO_CYLINDRICAL_CYLINDRICALSPECTRUM_H
