#ifndef WAVEGUIDE_SCENARIO_SIMPLESPECTRUMRANGE_H
#define WAVEGUIDE_SCENARIO_SIMPLESPECTRUMRANGE_H

#include "core/SceneObject.h"
#include "sdr/FrequencyBin.h"

#include <vector>

class SimpleSpectrumRange : public SceneObject
{
public:
    SimpleSpectrumRange(DisplayManager* display_manager, Primitive::Type type, const glm::vec3& world_coords, const glm::vec3& colour, const std::vector<sdr::FrequencyBin const*>& frequency_bins);
    virtual ~SimpleSpectrumRange();

    virtual void draw(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, bool use_colour = true);
    virtual void update(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, void* context);

protected:
    std::vector<sdr::FrequencyBin const*> frequency_bins_;
};

#endif //WAVEGUIDE_SCENARIO_SIMPLESPECTRUMRANGE_H
