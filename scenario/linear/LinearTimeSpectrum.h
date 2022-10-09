#ifndef WAVEGUIDE_SCENARIO_LINEAR_LINEARTIMESPECTRUM_H
#define WAVEGUIDE_SCENARIO_LINEAR_LINEARTIMESPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class LinearTimeSpectrum : public SimpleSpectrum {
public:
    LinearTimeSpectrum(insight::WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~LinearTimeSpectrum() = default;

    void run();

    // insight::InputHandler overrides
    void handleKeystroke(insight::WindowManager* window_manager, SDL_Event keystroke_event, GLfloat secs_since_last_renderloop) override;
    void handleMouse(insight::WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop) override;

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);
    void addInterestMarkerToBin(SimpleSpectrumRange *bin);

    void addSpectrumRanges(uint16_t slice_id, GLfloat secs_since_framequeue_started);

    uint16_t slices_;
    uint16_t current_slice_;
    uint64_t current_sweep_;

    // The mouse can be used to adjust the camera pointing vector in this scenario.
    bool tracking_mouse_;
    GLfloat mouse_start_x_, mouse_start_y_;
    GLfloat camera_pitch_degrees_;
    GLfloat camera_yaw_degrees_;
};


#endif //WAVEGUIDE_SCENARIO_LINEAR_LINEARTIMESPECTRUM_H
