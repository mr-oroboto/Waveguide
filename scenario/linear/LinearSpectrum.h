#ifndef WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H
#define WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class LinearSpectrum : public SimpleSpectrum
{
public:
    LinearSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~LinearSpectrum();

    void run();
    void clearInterestMarkers();

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);
    bool handleMouse(WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop);

    void addInterestMarkerToBin(SimpleSpectrumRange *bin);

    // Ray casts through spectrum bins on a mouse click to find zoom start / end points.
    SimpleSpectrumRange* findFirstIntersectedBin(GLuint mouse_x, GLuint mouse_y);

    // IDs of the text objects used on SimpleSpectrumRanges that have interest markers.
    std::vector<unsigned long> marked_bin_text_ids_;
};


#endif //WAVEGUIDE_SCENARIO_LINEAR_LINEARSPECTRUM_H
