#ifndef WAVEGUIDE_SCENARIO_CYLINDRICAL_CYLINDRICALSPECTRUM_H
#define WAVEGUIDE_SCENARIO_CYLINDRICAL_CYLINDRICALSPECTRUM_H

#include "scenario/SimpleSpectrum.h"

class CylindricalSpectrum : public SimpleSpectrum {
public:
    CylindricalSpectrum(insight::WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor = 1);
    ~CylindricalSpectrum() = default;

    void run();

    // insight::InputHandler overrides
    void handleKeystroke(insight::WindowManager* window_manager, SDL_Event keystroke_event, GLfloat secs_since_last_renderloop) override;
    void handleMouse(insight::WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop) override;

private:
    void updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame);

    void addSpectrumRanges(uint16_t ring_id, GLfloat secs_since_framequeue_started);

    uint16_t radius_;

    uint16_t rings_;
    uint16_t current_ring_;
    uint64_t current_sweep_;

    // The mouse can be used to adjust the camera pointing vector in this scenario.
    bool tracking_mouse_;
    GLfloat mouse_start_x_, mouse_start_y_;
    GLfloat camera_pitch_degrees_;
    GLfloat camera_yaw_degrees_;
};


#endif //WAVEGUIDE_SCENARIO_CYLINDRICAL_CYLINDRICALSPECTRUM_H
