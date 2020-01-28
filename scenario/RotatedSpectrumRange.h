#ifndef WAVEGUIDE_SCENARIO_ROTATEDSPECTRUMRANGE_H
#define WAVEGUIDE_SCENARIO_ROTATEDSPECTRUMRANGE_H

#include "SimpleSpectrumRange.h"

class RotatedSpectrumRange : public SimpleSpectrumRange
{
public:
    RotatedSpectrumRange(DisplayManager* display_manager, Primitive::Type type, uint16_t ring_id, uint64_t bin_id,
                       const glm::vec3& world_coords, double theta_offset, double phi_offset, double radius,
                       const glm::vec3& colour,
                       const std::vector<sdr::FrequencyBin const*>& frequency_bins);
    ~RotatedSpectrumRange();

    virtual void draw(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, bool use_colour = true);
    virtual void update(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, void* context);

private:
    uint64_t bin_id_;
    uint16_t ring_id_;

    double radius_;
    double theta_offset_;
    double rad_per_ring_;
};

#endif //WAVEGUIDE_SCENARIO_ROTATEDSPECTRUMRANGE_H
