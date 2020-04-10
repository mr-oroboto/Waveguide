#ifndef WAVEGUIDE_SCENARIO_SIMPLESPECTRUMRANGE_H
#define WAVEGUIDE_SCENARIO_SIMPLESPECTRUMRANGE_H

#include "core/SceneObject.h"
#include "sdr/FrequencyBin.h"

#include <vector>

class SimpleSpectrumRange : public SceneObject {
public:
    SimpleSpectrumRange(DisplayManager* display_manager, Primitive::Type type, uint16_t slice_id, uint64_t bin_id, const glm::vec3& world_coords, const glm::vec3& colour, const std::vector<sdr::FrequencyBin const*>& frequency_bins);
    virtual ~SimpleSpectrumRange() = default;

    virtual void draw(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, bool use_colour = true);
    virtual void update(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, void* context);

    float getAmplitude(bool refresh = false);

    uint64_t getFrequency();
    uint64_t getBinId();

    void setPicked(bool p) { picked_ = p; }

protected:
    std::vector<sdr::FrequencyBin const*> frequency_bins_;

    uint16_t slice_id_;
    uint64_t bin_id_;

    float amplitude_;

    bool picked_;
};

#endif //WAVEGUIDE_SCENARIO_SIMPLESPECTRUMRANGE_H
